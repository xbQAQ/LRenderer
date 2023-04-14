#include "UI.h"

static constexpr const char* title = "LRenderer";
static constexpr const char* cur_path = "./";
static constexpr const char* output_path = "./output/";

static void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
  exit(-1);
}

LRenderer::UI::UI(int w, int h)
    : width(w),
      height(h),
      image_texture(0),
      showWindow(true),
      windows(nullptr),
      use_blin_phong_shader(true),
      use_texture_shader(false),
      model_color(ImVec4(0.055, 0.733, 0.941, 1.0)),
      about_open(false) {
  picture_height = height * 4 / 5;
  picture_width = picture_height;
  pipeline = std::make_unique<PipeLine>(picture_width, picture_height);
}

void LRenderer::UI::Init() {
  pipeline->setObjPath(".\\models\\spot\\spot_triangulated_good.obj");
  pipeline->setTexturePath(".\\models\\spot\\spot_texture.png");
  pipeline->Init();
  pipeline->openMutltiThread();

  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    std::cout << "glfwInit error!" << std::endl;
    return;
  }

  const char* glsl_version = "#version 330";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

  windows = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (windows == nullptr) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
  }
  glfwMakeContextCurrent(windows);
  glfwSwapInterval(1);  // Enable vsync

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  // Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Multi-Viewport
                                                       // / Platform Windows

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform
  // windows can look identical to regular ones.
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(windows, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  auto fonts = io.Fonts->AddFontFromFileTTF("./NotoSansSC-Medium.otf", 18.0f);
  IM_ASSERT(fonts != NULL);
}

void LRenderer::UI::show() {
  while (!glfwWindowShouldClose(windows)) {
    pipeline->refresh();
    pipeline->draw();
    image = cv::Mat(picture_width, picture_height, CV_32FC3,
                    pipeline->raster->frameBuffer().data());
    image.convertTo(image, CV_8UC3, 1.0f);
    cv::cvtColor(image, image, cv::COLOR_RGB2BGRA);

    Mat2Texture(image, image_texture);

    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // window
    if (showWindow) {
      ImGui::Begin("LRenderer", &showWindow, ImGuiWindowFlags_MenuBar);

      showMenuBar();
      ImGui::Image((void*)(intptr_t)image_texture,
                   ImVec2(image.cols, image.rows));

      ImVec2 imageMin = ImGui::GetItemRectMin();
      ImVec2 imageMax = ImGui::GetItemRectMax();
      // 工具栏组
      ImGui::SetCursorPos(
          ImVec2(ImGui::GetWindowWidth() - 400, imageMin.y - 100));
      ImGui::BeginGroup();

      if (ImGui::Button("Reset")) {
        pipeline->reset();
      }

      ImGui::Checkbox("MultiThread", &multiThread);
      if (multiThread) {
        pipeline->openMutltiThread();
      } else {
        pipeline->closeMultiThread();
      }

      ImGui::Checkbox("Back Clip", &back_clip);
      if (back_clip) {
        pipeline->openBackClip();
      } else {
        pipeline->closeBackClip();
      }

      ImGui::Checkbox("Line Mode", &line_mode);
      if (line_mode) {
        pipeline->openLineMode();
      } else {
        pipeline->closeLineMode();
      }

      static int AA_mode = SSAA;
      if (ImGui::RadioButton("SSAA", AA_mode == SSAA)) {
        AA_mode = SSAA;
        pipeline->openSSAA();
      }
      if (ImGui::RadioButton("MSAA", AA_mode == MSAA)) {
        AA_mode = MSAA;
        pipeline->openMSAA();
      }
      if (ImGui::RadioButton("Normal", AA_mode == NORMAL)) {
        AA_mode = NORMAL;
        pipeline->openNormal();
      }

      static int freq_mode = _4X;
      const char* elems_names[FREQSIZE] = {"4X", "9X", "16X"};
      const char* elem_name = (freq_mode >= 0 && freq_mode < FREQSIZE)
                                  ? elems_names[freq_mode]
                                  : "Unknown";
      ImGui::SetNextItemWidth(300);
      if (ImGui::SliderInt("Freq", &freq_mode, 0, FREQSIZE - 1, elem_name)) {
        if (freq_mode == _4X) {
          pipeline->raster->set4xFreq();
        } else if (freq_mode == _9X) {
          pipeline->raster->set9xFreq();
        } else if (freq_mode == _16X) {
          pipeline->raster->set16xFreq();
        }
      }

      static int shader_mode = 0;
      if (ImGui::RadioButton("Texture Shader", shader_mode == TEXTURESHADER)) {
        shader_mode = TEXTURESHADER;
        pipeline->setTextureShader();
      }
      if (ImGui::RadioButton("BlinPhong Shader",
                             shader_mode == BLINPHONGSHADER)) {
        shader_mode = BLINPHONGSHADER;
        pipeline->setBlinPhongShader();
      }

      ImGui::ColorEdit4(
          "model_color", (float*)&model_color,
          ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
      ImGui::SameLine();
      ImGui::Text("Model Color");
      pipeline->changeColor(
          Eigen::Vector3f(model_color.x, model_color.y, model_color.z));

      ImGui::PushItemWidth(300);
      static float fov = 45.0;
      if (ImGui::SliderFloat("FOV", &fov, 0.1, 90.0)) {
        pipeline->camera.updateFov(fov);
      }

      static float znear = 0.1;
      if (ImGui::SliderFloat("Z Near", &znear, 0.1, 50.0)) {
        pipeline->camera.changeNear(znear);
      }

      static float zfar = 50.0;
      if (ImGui::SliderFloat("Z Far", &zfar, 20.0, 50.0)) {
        pipeline->camera.changeFar(zfar);
      }
      ImGui::PopItemWidth();

      inputProcess();

      ImGui::Text("Application average %.1f FPS", ImGui::GetIO().Framerate);

      ImGui::EndGroup();
      ImGui::End();
    }
    if (about_open) {
      showAbout();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(windows, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    const ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(windows);
  }
}

LRenderer::UI::~UI() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(windows);
  glfwTerminate();
}

void LRenderer::UI::Mat2Texture(const cv::Mat& image, GLuint& imageTexture) {
  if (image.empty()) {
    std::cout << "image is empty! " << std::endl;
    return;
  } else {
    // generate texture using GL commands
    glGenTextures(1, &imageTexture);
    glBindTexture(GL_TEXTURE_2D, imageTexture);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_BGRA,
                 GL_UNSIGNED_BYTE, image.data);
  }
}

inline void LRenderer::UI::inputProcess() {
  if (ImGui::IsKeyDown(ImGuiKey_A)) {  // A
    pipeline->camera.moveLeft();
  } else if (ImGui::IsKeyDown(ImGuiKey_D)) {  // D
    pipeline->camera.moveRight();
  } else if (ImGui::IsKeyDown(ImGuiKey_W)) {  // W
    pipeline->camera.moveUp();
  } else if (ImGui::IsKeyDown(ImGuiKey_S)) {  // S
    pipeline->camera.moveDown();
  } else if (ImGui::IsKeyDown(ImGuiKey_Q)) {  // Q
    pipeline->camera.moveForward();
  } else if (ImGui::IsKeyDown(ImGuiKey_E)) {  // E
    pipeline->camera.moveBack();
  } else if (ImGui::GetIO().MouseWheel > 0.0f) {  //滚轮上滑
    pipeline->camera.magnification();
  } else if (ImGui::GetIO().MouseWheel < 0.0f) {  //滚轮下滑
    pipeline->camera.shrinks();
  } else if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {  // <=
    pipeline->camera.rotatePositiveDirectionAroundY();
  } else if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {  // =>
    pipeline->camera.rotateNegitiveDirectionAroundY();
  } else if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {  // ⬆
    pipeline->camera.rotatePositiveDirectionAroundX();
  } else if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {  // ⬇
    pipeline->camera.rotateNegitiveDirectionAroundX();
  } else if (ImGui::IsKeyDown(ImGuiKey_1)) {  // 大键盘1
    pipeline->camera.rotatePositiveDirectionAroundZ();
  } else if (ImGui::IsKeyDown(ImGuiKey_3)) {  //大键盘3
    pipeline->camera.rotateNegitiveDirectionAroundZ();
  }
}

void LRenderer::UI::showMenuBar() {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open .obj file")) {
        OPENFILENAME ofn;
        TCHAR szFile[MAX_PATH];

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = TEXT("OBJ Files\0*.obj\0All Files\0*.*\0");
        ofn.nFilterIndex = 1;
        ofn.lpstrInitialDir = cur_path;  //将默认路径设置为当前项目的路径
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn)) {
          std::string path = szFile;
          pipeline->changeObjPath(path);
        }
      }
      if (ImGui::MenuItem("Open texture file")) {
        OPENFILENAME ofn;
        TCHAR szFile[MAX_PATH];

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = TEXT("PNG/JPG Files\0*.png;*.jpg\0All Files\0*.*\0");
        ofn.nFilterIndex = 1;
        ofn.lpstrInitialDir = cur_path;  //将默认路径设置为当前项目的路径
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn)) {
          std::string path = szFile;
          pipeline->changeTexturePath(path);
        }
      }
      if (ImGui::MenuItem("Save")) {
        static int index = 0;
        const std::string outputname =
            "./output/output" + std::to_string(index++) + ".png";
        if (_access(output_path, 0) == -1) {
          if (errno == EACCES || errno == EINVAL) {
            std::cerr << "Valid Path" << std::endl;
            return;
          } else {
            // 如果目录不存在
            if (_mkdir(output_path) == 0 || errno == EEXIST) {
              cv::imwrite(outputname, image);
            } else {
              std::cerr << "Can't find outpath" << std::endl;
            }
          }
        } else {
          cv::imwrite(outputname, image);
        }
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Style")) {
      if (ImGui::MenuItem("Dark")) {
        ImGui::StyleColorsDark();
        pipeline->raster->changeBackgroudColor(Eigen::Vector3f(0, 0, 0));
      } else if (ImGui::MenuItem("Light")) {
        ImGui::StyleColorsLight();
        pipeline->raster->changeBackgroudColor(Eigen::Vector3f(255, 255, 255));
      }
      ImGui::EndMenu();
    }
    if (ImGui::MenuItem("About")) {
      about_open = true;
    }
    ImGui::EndMenuBar();
  }
}

void LRenderer::UI::showAbout() {
  ImGui::Begin("About LRenderer", &about_open,
               ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::Text("Github: https://github.com/xbQAQ/LRenderer");
  ImGui::Separator();
  ImGui::Text("A: Move Left\tD: Move Right\tW: Move Up\tS: Move Down\tQ: Move Forward\tE: moveBack");
  ImGui::Text("Up: rotate positive direction around X");
  ImGui::Text("Down: rotate negitive direction around X");
  ImGui::Text("Left: rotate positive direction around Y");
  ImGui::Text("Right: rotate negitive direction around Y");
  ImGui::Text("1: rotate positive direction around Z");
  ImGui::Text("3: rotate negitive direction around Z");
  ImGui::Text("mouse wheel up: magnification");
  ImGui::Text("mouse wheel down: shrinks");
  ImGui::End();
}
