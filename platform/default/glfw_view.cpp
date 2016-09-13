#include <mbgl/platform/default/glfw_view.hpp>
#include <mbgl/annotation/annotation.hpp>
#include <mbgl/sprite/sprite_image.hpp>
#include <mbgl/style/transition_options.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/gl/gl_values.hpp>
#include <mbgl/gl/gl_helper.hpp>
#include <mbgl/platform/log.hpp>
#include <mbgl/platform/platform.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/map/camera.hpp>

#include <cassert>
#include <cstdlib>

void glfwError(int error, const char *description) {
    mbgl::Log::Error(mbgl::Event::OpenGL, "GLFW error (%i): %s", error, description);
    assert(false);
}

GLFWView::GLFWView(bool fullscreen_, bool benchmark_)
    : fullscreen(fullscreen_), benchmark(benchmark_) {
    glfwSetErrorCallback(glfwError);

    std::srand(std::time(nullptr));

    if (!glfwInit()) {
        mbgl::Log::Error(mbgl::Event::OpenGL, "failed to initialize glfw");
        exit(1);
    }

    GLFWmonitor *monitor = nullptr;
    if (fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        auto videoMode = glfwGetVideoMode(monitor);
        width = videoMode->width;
        height = videoMode->height;
    }

#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

#ifdef GL_ES_VERSION_2_0
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);

    window = glfwCreateWindow(width, height, "Mapbox GL", monitor, nullptr);
    if (!window) {
        glfwTerminate();
        mbgl::Log::Error(mbgl::Event::OpenGL, "failed to initialize window");
        exit(1);
    }

    glfwSetWindowUserPointer(window, this);
    glfwMakeContextCurrent(window);
    if (benchmark) {
        // Disables vsync on platforms that support it.
        glfwSwapInterval(0);
    } else {
        glfwSwapInterval(1);
    }


    glfwSetCursorPosCallback(window, onMouseMove);
    glfwSetMouseButtonCallback(window, onMouseClick);
    glfwSetWindowSizeCallback(window, onWindowResize);
    glfwSetFramebufferSizeCallback(window, onFramebufferResize);
    glfwSetScrollCallback(window, onScroll);
    glfwSetKeyCallback(window, onKey);

    mbgl::gl::InitializeExtensions(glfwGetProcAddress);

    glfwGetWindowSize(window, &width, &height);
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    pixelRatio = static_cast<float>(fbWidth) / width;

    glfwMakeContextCurrent(nullptr);

    printf("\n");
    printf("================================================================================\n");
    printf("\n");
    printf("- Press `S` to cycle through bundled styles\n");
    printf("- Press `X` to reset the transform\n");
    printf("- Press `N` to reset north\n");
    printf("- Press `R` to toggle any available `night` style class\n");
    printf("- Press `Z` to cycle through north orientations\n");
    printf("- Prezz `X` to cycle through the viewport modes\n");
    printf("- Press `A` to cycle through Mapbox offices in the world + dateline monument\n");
    printf("- Press `B` to cycle through the color, stencil, and depth buffer\n");
    printf("\n");
    printf("- Press `1` through `6` to add increasing numbers of point annotations for testing\n");
    printf("- Press `7` through `0` to add increasing numbers of shape annotations for testing\n");
    printf("\n");
    printf("- Press `Q` to remove annotations\n");
    printf("- Press `P` to add a random custom runtime imagery annotation\n");
    printf("- Press `L` to add a random line annotation\n");
    printf("- Press `W` to pop the last-added annotation off\n");
    printf("\n");
    printf("- `Control` + mouse drag to rotate\n");
    printf("- `Shift` + mouse drag to tilt\n");
    printf("\n");
    printf("- Press `Tab` to cycle through the map debug options\n");
    printf("- Press `Esc` to quit\n");
    printf("\n");
    printf("================================================================================\n");
    printf("\n");
}

GLFWView::~GLFWView() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GLFWView::initialize(mbgl::Map *map_) {
    View::initialize(map_);
    map->addAnnotationIcon("default_marker", makeSpriteImage(22, 22, 1));
}

void GLFWView::onKey(GLFWwindow *window, int key, int /*scancode*/, int action, int mods) {
    GLFWView *view = reinterpret_cast<GLFWView *>(glfwGetWindowUserPointer(window));

    if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        case GLFW_KEY_TAB:
            view->map->cycleDebugOptions();
            break;
        case GLFW_KEY_X:
            if (!mods)
            {
                static std::pair<int,int> annotationIDs = {0,0};
                static const std::pair<int, int> empty={0,0};
                if(annotationIDs != empty)
                {
                    view->map->removeAnnotation(annotationIDs.first);
                    view->map->removeAnnotation(annotationIDs.second);
                }

                mbgl::LineString<double> lineString;
                lineString.push_back({-122.1507, 37.39459});
                lineString.push_back({-122.15055, 37.3943});
                lineString.push_back({-122.15076, 37.39423});
                lineString.push_back({-122.15098, 37.39416});
                lineString.push_back({-122.15153, 37.39517});
                lineString.push_back({-122.1516, 37.3953});
                lineString.push_back({-122.15219, 37.39552});
                lineString.push_back({-122.15216, 37.39558});
                lineString.push_back({-122.15207, 37.39564});
                lineString.push_back({-122.15193, 37.39575});
                lineString.push_back({-122.15182, 37.39584});
                lineString.push_back({-122.15181, 37.39588});
                lineString.push_back({-122.15165, 37.39622});
                lineString.push_back({-122.15275, 37.39662});
                lineString.push_back({-122.15307, 37.39672});
                lineString.push_back({-122.15335, 37.39677});
                lineString.push_back({-122.15337, 37.39677});
                lineString.push_back({-122.15364, 37.3968});
                lineString.push_back({-122.1539, 37.39681});
                lineString.push_back({-122.15416, 37.39679});
                lineString.push_back({-122.15442, 37.39676});
                lineString.push_back({-122.15536, 37.39662});
                lineString.push_back({-122.15567, 37.39659});
                lineString.push_back({-122.15597, 37.39659});
                lineString.push_back({-122.15628, 37.39661});
                lineString.push_back({-122.15661, 37.39668});
                lineString.push_back({-122.15692, 37.39678});
                lineString.push_back({-122.15718, 37.39691});
                lineString.push_back({-122.15742, 37.39706});
                lineString.push_back({-122.1576, 37.3972});
                lineString.push_back({-122.15783, 37.3974});
                lineString.push_back({-122.1579, 37.39748});
                lineString.push_back({-122.15801, 37.3976});
                lineString.push_back({-122.15897, 37.39706});
                lineString.push_back({-122.1595, 37.39677});
                lineString.push_back({-122.15992, 37.39652});
                lineString.push_back({-122.16018, 37.39636});
                lineString.push_back({-122.16039, 37.39619});
                lineString.push_back({-122.16057, 37.39605});
                lineString.push_back({-122.16071, 37.39589});
                lineString.push_back({-122.16084, 37.39572});
                lineString.push_back({-122.16099, 37.39548});
                lineString.push_back({-122.16109, 37.39524});
                lineString.push_back({-122.16128, 37.39463});
                lineString.push_back({-122.16138, 37.39438});
                lineString.push_back({-122.16151, 37.39417});
                lineString.push_back({-122.16167, 37.39397});
                lineString.push_back({-122.16179, 37.39382});
                lineString.push_back({-122.16191, 37.39372});
                lineString.push_back({-122.16221, 37.39347});
                lineString.push_back({-122.16238, 37.39333});
                lineString.push_back({-122.1625, 37.39319});
                lineString.push_back({-122.16264, 37.39299});
                lineString.push_back({-122.16271, 37.39287});
                lineString.push_back({-122.16278, 37.39272});
                lineString.push_back({-122.16285, 37.39258});
                lineString.push_back({-122.1629, 37.39243});
                lineString.push_back({-122.16292, 37.39233});
                lineString.push_back({-122.16299, 37.39201});
                lineString.push_back({-122.16301, 37.39195});
                lineString.push_back({-122.16319, 37.39115});
                lineString.push_back({-122.16327, 37.39079});
                lineString.push_back({-122.16361, 37.38948});
                lineString.push_back({-122.16359, 37.38937});
                lineString.push_back({-122.16364, 37.38914});
                lineString.push_back({-122.16374, 37.38864});
                lineString.push_back({-122.16381, 37.38832});
                lineString.push_back({-122.16385, 37.38818});
                lineString.push_back({-122.16387, 37.38806});
                lineString.push_back({-122.16392, 37.38781});
                lineString.push_back({-122.16395, 37.38744});
                lineString.push_back({-122.16412, 37.38701});
                lineString.push_back({-122.1645, 37.38653});
                lineString.push_back({-122.16486, 37.38616});
                lineString.push_back({-122.16516, 37.38581});
                lineString.push_back({-122.1653, 37.38566});
                lineString.push_back({-122.1654, 37.38545});
                lineString.push_back({-122.16565, 37.38491});
                lineString.push_back({-122.16573, 37.3849});
                lineString.push_back({-122.16737, 37.38496});
                lineString.push_back({-122.16832, 37.38502});
                lineString.push_back({-122.16924, 37.38524});
                lineString.push_back({-122.16983, 37.38546});
                lineString.push_back({-122.17026, 37.38564});
                lineString.push_back({-122.17108, 37.38597});
                lineString.push_back({-122.17226, 37.38631});
                lineString.push_back({-122.17226, 37.38631});
                lineString.push_back({-122.17393, 37.38679});
                lineString.push_back({-122.17478, 37.38689});
                lineString.push_back({-122.17644, 37.38706});
                lineString.push_back({-122.17833, 37.38678});
                lineString.push_back({-122.17853, 37.38674});
                lineString.push_back({-122.17872, 37.38664});
                lineString.push_back({-122.17885, 37.38652});
                lineString.push_back({-122.17985, 37.38569});
                lineString.push_back({-122.18025, 37.38545});
                lineString.push_back({-122.18053, 37.38514});
                lineString.push_back({-122.18066, 37.38498});
                lineString.push_back({-122.18092, 37.38458});
                lineString.push_back({-122.18108, 37.38418});
                lineString.push_back({-122.18127, 37.38314});
                lineString.push_back({-122.18154, 37.38263});
                lineString.push_back({-122.18166, 37.38207});
                lineString.push_back({-122.18174, 37.38168});
                lineString.push_back({-122.18181, 37.38143});
                lineString.push_back({-122.18202, 37.38122});
                lineString.push_back({-122.18229, 37.38107});
                lineString.push_back({-122.18246, 37.38101});
                lineString.push_back({-122.18253, 37.38099});
                lineString.push_back({-122.18384, 37.38099});
                lineString.push_back({-122.1841, 37.38105});
                lineString.push_back({-122.18429, 37.38116});
                lineString.push_back({-122.18447, 37.38138});
                lineString.push_back({-122.18454, 37.38161});
                lineString.push_back({-122.18457, 37.38193});
                lineString.push_back({-122.18463, 37.38208});
                lineString.push_back({-122.18473, 37.38219});
                lineString.push_back({-122.18509, 37.38239});
                lineString.push_back({-122.18553, 37.38272});
                lineString.push_back({-122.18581, 37.38299});
                lineString.push_back({-122.18595, 37.38313});
                lineString.push_back({-122.18647, 37.38378});
                lineString.push_back({-122.18674, 37.38398});
                lineString.push_back({-122.18715, 37.38423});
                lineString.push_back({-122.18751, 37.38451});
                lineString.push_back({-122.18787, 37.38471});
                lineString.push_back({-122.18816, 37.38476});
                lineString.push_back({-122.18938, 37.38479});
                lineString.push_back({-122.18961, 37.38475});
                lineString.push_back({-122.19025, 37.38437});
                lineString.push_back({-122.1922, 37.38332});
                lineString.push_back({-122.19247, 37.38321});
                lineString.push_back({-122.19284, 37.38305});
                lineString.push_back({-122.19322, 37.38285});
                lineString.push_back({-122.19325, 37.3828});
                lineString.push_back({-122.19345, 37.38254});
                lineString.push_back({-122.1938, 37.3824});
                lineString.push_back({-122.19433, 37.38194});
                lineString.push_back({-122.19457, 37.38167});
                lineString.push_back({-122.19481, 37.38133});

                annotationIDs = {view->map->addAnnotation(mbgl::StyleSourcedAnnotation { lineString, std::string("route-bg") }),
                                 view->map->addAnnotation(mbgl::StyleSourcedAnnotation { lineString, std::string("route") })};
            }
            break;
        case GLFW_KEY_S:
            if (view->changeStyleCallback)
                view->changeStyleCallback();
            break;
        case GLFW_KEY_R:
            if (!mods) {
                static const mbgl::style::TransitionOptions transition { { mbgl::Milliseconds(300) } };
                view->map->setTransitionOptions(transition);
                if (view->map->hasClass("night")) {
                    view->map->removeClass("night");
                } else {
                    view->map->addClass("night");
                }
            }
            break;
        case GLFW_KEY_B: {
            auto debug = view->map->getDebug();
            if (debug & mbgl::MapDebugOptions::StencilClip) {
                debug &= ~mbgl::MapDebugOptions::StencilClip;
                debug |= mbgl::MapDebugOptions::DepthBuffer;
                printf("Depth debug\n");
            } else if (debug & mbgl::MapDebugOptions::DepthBuffer) {
                debug &= ~mbgl::MapDebugOptions::DepthBuffer;
            } else {
                debug |= mbgl::MapDebugOptions::StencilClip;
                printf("Stencil Clip Debug\n");
            }
            view->map->setDebug(debug);
        } break;
        case GLFW_KEY_N:
            if (!mods)
                view->map->resetNorth();
            break;
        case GLFW_KEY_Z:
            view->nextOrientation();
            break;
        case GLFW_KEY_Q:
            view->clearAnnotations();
            break;
        case GLFW_KEY_P:
            view->addRandomCustomPointAnnotations(1);
            break;
        case GLFW_KEY_L:
            view->addRandomLineAnnotations(1);
            break;
        case GLFW_KEY_A: {
            // XXX Fix precision loss in flyTo:
            // https://github.com/mapbox/mapbox-gl-native/issues/4298
            static const std::vector<mbgl::LatLng> places = {
                mbgl::LatLng { -16.796665, -179.999983 },   // Dateline monument
                mbgl::LatLng { 12.9810542, 77.6345551 },    // Mapbox Bengaluru, India
                mbgl::LatLng { -13.15607,-74.21773 },       // Mapbox Peru
                mbgl::LatLng { 37.77572, -122.4158818 },    // Mapbox SF, USA
                mbgl::LatLng { 38.91318,-77.03255 },        // Mapbox DC, USA
            };
            static size_t nextPlace = 0;
            mbgl::CameraOptions cameraOptions;
            cameraOptions.center = places[nextPlace++];
            cameraOptions.zoom = 20;
            cameraOptions.pitch = 30;

            mbgl::AnimationOptions animationOptions(mbgl::Seconds(10));
            view->map->flyTo(cameraOptions, animationOptions);
            nextPlace = nextPlace % places.size();
        } break;
        }
    }

    if (action == GLFW_RELEASE || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_W: view->popAnnotation(); break;
        case GLFW_KEY_1: view->addRandomPointAnnotations(1); break;
        case GLFW_KEY_2: view->addRandomPointAnnotations(10); break;
        case GLFW_KEY_3: view->addRandomPointAnnotations(100); break;
        case GLFW_KEY_4: view->addRandomPointAnnotations(1000); break;
        case GLFW_KEY_5: view->addRandomPointAnnotations(10000); break;
        case GLFW_KEY_6: view->addRandomPointAnnotations(100000); break;
        case GLFW_KEY_7: view->addRandomShapeAnnotations(1); break;
        case GLFW_KEY_8: view->addRandomShapeAnnotations(10); break;
        case GLFW_KEY_9: view->addRandomShapeAnnotations(100); break;
        case GLFW_KEY_0: view->addRandomShapeAnnotations(1000); break;
        }
    }
}

mbgl::Color GLFWView::makeRandomColor() const {
    const float r = 1.0f * (float(std::rand()) / RAND_MAX);
    const float g = 1.0f * (float(std::rand()) / RAND_MAX);
    const float b = 1.0f * (float(std::rand()) / RAND_MAX);
    return { r, g, b, 1.0f };
}

mbgl::Point<double> GLFWView::makeRandomPoint() const {
    const double x = width * double(std::rand()) / RAND_MAX;
    const double y = height * double(std::rand()) / RAND_MAX;
    mbgl::LatLng latLng = map->latLngForPixel({ x, y });
    return { latLng.longitude, latLng.latitude };
}

std::shared_ptr<const mbgl::SpriteImage>
GLFWView::makeSpriteImage(int width, int height, float pixelRatio) {
    const int r = 255 * (double(std::rand()) / RAND_MAX);
    const int g = 255 * (double(std::rand()) / RAND_MAX);
    const int b = 255 * (double(std::rand()) / RAND_MAX);

    const int w = std::ceil(pixelRatio * width);
    const int h = std::ceil(pixelRatio * height);

    mbgl::PremultipliedImage image(w, h);
    auto data = reinterpret_cast<uint32_t*>(image.data.get());
    const int dist = (w / 2) * (w / 2);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            const int dx = x - w / 2;
            const int dy = y - h / 2;
            const int diff = dist - (dx * dx + dy * dy);
            if (diff > 0) {
                const int a = std::min(0xFF, diff) * 0xFF / dist;
                // Premultiply the rgb values with alpha
                data[w * y + x] =
                    (a << 24) | ((a * r / 0xFF) << 16) | ((a * g / 0xFF) << 8) | (a * b / 0xFF);
            }
        }
    }

    return std::make_shared<mbgl::SpriteImage>(std::move(image), pixelRatio);
}

void GLFWView::nextOrientation() {
    using NO = mbgl::NorthOrientation;
    switch (map->getNorthOrientation()) {
        case NO::Upwards: map->setNorthOrientation(NO::Rightwards); break;
        case NO::Rightwards: map->setNorthOrientation(NO::Downwards); break;
        case NO::Downwards: map->setNorthOrientation(NO::Leftwards); break;
        default: map->setNorthOrientation(NO::Upwards); break;
    }
}

void GLFWView::addRandomCustomPointAnnotations(int count) {
    for (int i = 0; i < count; i++) {
        static int spriteID = 1;
        const auto name = std::string{ "marker-" } + mbgl::util::toString(spriteID++);
        map->addAnnotationIcon(name, makeSpriteImage(22, 22, 1));
        spriteIDs.push_back(name);
        annotationIDs.push_back(map->addAnnotation(mbgl::SymbolAnnotation { makeRandomPoint(), name }));
    }
}

void GLFWView::addRandomPointAnnotations(int count) {
    for (int i = 0; i < count; ++i) {
        annotationIDs.push_back(map->addAnnotation(mbgl::SymbolAnnotation { makeRandomPoint(), "default_marker" }));
    }
}

void GLFWView::addRandomLineAnnotations(int count) {
    for (int i = 0; i < count; ++i) {
        mbgl::LineString<double> lineString;
        for (int j = 0; j < 3; ++j) {
            lineString.push_back(makeRandomPoint());
        }
        annotationIDs.push_back(map->addAnnotation(mbgl::LineAnnotation { lineString, 1.0f, 2.0f, { makeRandomColor() } }));
    }
}

void GLFWView::addRandomShapeAnnotations(int count) {
    for (int i = 0; i < count; ++i) {
        mbgl::Polygon<double> triangle;
        triangle.push_back({ makeRandomPoint(), makeRandomPoint(), makeRandomPoint() });
        annotationIDs.push_back(map->addAnnotation(mbgl::FillAnnotation { triangle, 0.5f, { makeRandomColor() }, { makeRandomColor() } }));
    }
}

void GLFWView::clearAnnotations() {
    for (const auto& id : annotationIDs) {
        map->removeAnnotation(id);
    }

    annotationIDs.clear();
}

void GLFWView::popAnnotation() {
    if (annotationIDs.empty()) {
        return;
    }

    map->removeAnnotation(annotationIDs.back());
    annotationIDs.pop_back();
}

void GLFWView::onScroll(GLFWwindow *window, double /*xOffset*/, double yOffset) {
    GLFWView *view = reinterpret_cast<GLFWView *>(glfwGetWindowUserPointer(window));
    double delta = yOffset * 40;

    bool isWheel = delta != 0 && std::fmod(delta, 4.000244140625) == 0;

    double absDelta = delta < 0 ? -delta : delta;
    double scale = 2.0 / (1.0 + std::exp(-absDelta / 100.0));

    // Make the scroll wheel a bit slower.
    if (!isWheel) {
        scale = (scale - 1.0) / 2.0 + 1.0;
    }

    // Zooming out.
    if (delta < 0 && scale != 0) {
        scale = 1.0 / scale;
    }

    view->map->scaleBy(scale, mbgl::ScreenCoordinate { view->lastX, view->lastY });
}

void GLFWView::onWindowResize(GLFWwindow *window, int width, int height) {
    GLFWView *view = reinterpret_cast<GLFWView *>(glfwGetWindowUserPointer(window));
    view->width = width;
    view->height = height;

    view->map->update(mbgl::Update::Dimensions);
}

void GLFWView::onFramebufferResize(GLFWwindow *window, int width, int height) {
    GLFWView *view = reinterpret_cast<GLFWView *>(glfwGetWindowUserPointer(window));
    view->fbWidth = width;
    view->fbHeight = height;

    view->map->update(mbgl::Update::Repaint);
}

void GLFWView::onMouseClick(GLFWwindow *window, int button, int action, int modifiers) {
    GLFWView *view = reinterpret_cast<GLFWView *>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_RIGHT ||
        (button == GLFW_MOUSE_BUTTON_LEFT && modifiers & GLFW_MOD_CONTROL)) {
        view->rotating = action == GLFW_PRESS;
        view->map->setGestureInProgress(view->rotating);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && (modifiers & GLFW_MOD_SHIFT)) {
        view->pitching = action == GLFW_PRESS;
        view->map->setGestureInProgress(view->pitching);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT) {
        view->tracking = action == GLFW_PRESS;
        view->map->setGestureInProgress(view->tracking);

        if (action == GLFW_RELEASE) {
            double now = glfwGetTime();
            if (now - view->lastClick < 0.4 /* ms */) {
                if (modifiers & GLFW_MOD_SHIFT) {
                    view->map->scaleBy(0.5, mbgl::ScreenCoordinate { view->lastX, view->lastY }, mbgl::Milliseconds(500));
                } else {
                    view->map->scaleBy(2.0, mbgl::ScreenCoordinate { view->lastX, view->lastY }, mbgl::Milliseconds(500));
                }
            }
            view->lastClick = now;
        }
    }
}

void GLFWView::onMouseMove(GLFWwindow *window, double x, double y) {
    GLFWView *view = reinterpret_cast<GLFWView *>(glfwGetWindowUserPointer(window));
    if (view->tracking) {
        double dx = x - view->lastX;
        double dy = y - view->lastY;
        if (dx || dy) {
            view->map->setLatLng(
                    view->map->latLngForPixel(mbgl::ScreenCoordinate(x - dx, y - dy)),
                    mbgl::ScreenCoordinate(x, y));
        }
    } else if (view->rotating) {
        view->map->rotateBy({ view->lastX, view->lastY }, { x, y });
    } else if (view->pitching) {
        const double dy = y - view->lastY;
        if (dy) {
            view->map->setPitch(view->map->getPitch() - dy / 2);
        }
    }
    view->lastX = x;
    view->lastY = y;
}

void GLFWView::run() {
    auto callback = [&] {
        if (glfwWindowShouldClose(window)) {
            runLoop.stop();
            return;
        }

        glfwPollEvents();

        if (dirty) {
            const double started = glfwGetTime();

            glfwMakeContextCurrent(window);
            glViewport(0, 0, fbWidth, fbHeight);

            map->render();

            glfwSwapBuffers(window);

            report(1000 * (glfwGetTime() - started));
            if (benchmark) {
                map->update(mbgl::Update::Repaint);
            }

            dirty = false;
        }
    };

    frameTick.start(mbgl::Duration::zero(), mbgl::Milliseconds(1000 / 60), callback);
#if defined(__APPLE__)
    while (!glfwWindowShouldClose(window)) runLoop.run();
#else
    runLoop.run();
#endif
}

float GLFWView::getPixelRatio() const {
    return pixelRatio;
}

std::array<uint16_t, 2> GLFWView::getSize() const {
    return {{ static_cast<uint16_t>(width), static_cast<uint16_t>(height) }};
}

std::array<uint16_t, 2> GLFWView::getFramebufferSize() const {
    return {{ static_cast<uint16_t>(fbWidth), static_cast<uint16_t>(fbHeight) }};
}

void GLFWView::activate() {
    glfwMakeContextCurrent(window);
}

void GLFWView::deactivate() {
    glfwMakeContextCurrent(nullptr);
}

void GLFWView::invalidate() {
    dirty = true;
    glfwPostEmptyEvent();
}

void GLFWView::report(float duration) {
    frames++;
    frameTime += duration;

    const double currentTime = glfwGetTime();
    if (currentTime - lastReported >= 1) {
        frameTime /= frames;
        mbgl::Log::Info(mbgl::Event::OpenGL, "Frame time: %6.2fms (%6.2f fps)", frameTime,
            1000 / frameTime);
        frames = 0;
        frameTime = 0;
        lastReported = currentTime;
    }
}

void GLFWView::setChangeStyleCallback(std::function<void()> callback) {
    changeStyleCallback = callback;
}

void GLFWView::setShouldClose() {
    glfwSetWindowShouldClose(window, true);
    glfwPostEmptyEvent();
}

void GLFWView::setWindowTitle(const std::string& title) {
    glfwSetWindowTitle(window, (std::string { "Mapbox GL: " } + title).c_str());
}

namespace mbgl {
namespace platform {

#ifndef GL_ES_VERSION_2_0
void showDebugImage(std::string name, const char *data, size_t width, size_t height) {
    glfwInit();

    static GLFWwindow *debugWindow = nullptr;
    if (!debugWindow) {
        debugWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        if (!debugWindow) {
            glfwTerminate();
            fprintf(stderr, "Failed to initialize window\n");
            exit(1);
        }
    }

    GLFWwindow *currentWindow = glfwGetCurrentContext();

    glfwSetWindowSize(debugWindow, width, height);
    glfwMakeContextCurrent(debugWindow);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(debugWindow, &fbWidth, &fbHeight);
    float scale = static_cast<float>(fbWidth) / static_cast<float>(width);

    {
        gl::Preserve<gl::PixelZoom> pixelZoom;
        gl::Preserve<gl::RasterPos> rasterPos;

        MBGL_CHECK_ERROR(glPixelZoom(scale, -scale));
        MBGL_CHECK_ERROR(glRasterPos2f(-1.0f, 1.0f));
        MBGL_CHECK_ERROR(glDrawPixels(width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, data));
    }

    glfwSwapBuffers(debugWindow);

    glfwMakeContextCurrent(currentWindow);
}

void showColorDebugImage(std::string name, const char *data, size_t logicalWidth, size_t logicalHeight, size_t width, size_t height) {
    glfwInit();

    static GLFWwindow *debugWindow = nullptr;
    if (!debugWindow) {
        debugWindow = glfwCreateWindow(logicalWidth, logicalHeight, name.c_str(), nullptr, nullptr);
        if (!debugWindow) {
            glfwTerminate();
            fprintf(stderr, "Failed to initialize window\n");
            exit(1);
        }
    }

    GLFWwindow *currentWindow = glfwGetCurrentContext();

    glfwSetWindowSize(debugWindow, logicalWidth, logicalHeight);
    glfwMakeContextCurrent(debugWindow);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(debugWindow, &fbWidth, &fbHeight);
    float xScale = static_cast<float>(fbWidth) / static_cast<float>(width);
    float yScale = static_cast<float>(fbHeight) / static_cast<float>(height);

    {
        gl::Preserve<gl::ClearColor> clearColor;
        gl::Preserve<gl::Blend> blend;
        gl::Preserve<gl::BlendFunc> blendFunc;
        gl::Preserve<gl::PixelZoom> pixelZoom;
        gl::Preserve<gl::RasterPos> rasterPos;

        MBGL_CHECK_ERROR(glClearColor(0.8, 0.8, 0.8, 1));
        MBGL_CHECK_ERROR(glClear(GL_COLOR_BUFFER_BIT));
        MBGL_CHECK_ERROR(glEnable(GL_BLEND));
        MBGL_CHECK_ERROR(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        MBGL_CHECK_ERROR(glPixelZoom(xScale, -yScale));
        MBGL_CHECK_ERROR(glRasterPos2f(-1.0f, 1.0f));
        MBGL_CHECK_ERROR(glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, data));
    }

    glfwSwapBuffers(debugWindow);

    glfwMakeContextCurrent(currentWindow);
}
#endif

} // namespace platform
} // namespace mbgl
