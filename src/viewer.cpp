#include "viewer.h"

Viewer::Viewer(int argc, char *argv[])
{
    std::cout << "Initializing device and sensors" << std::endl;
    // initialize global variables
    exitFlag = false;
    saveMemory = false;
    saveDisk = false;
    save_depth = true;
    save_rgb = true;
    save_pcd = false;
    save_rgbd = false;
    save_yml = false;
    img_type = ".png";
    limitx_min = 0;
    limitx_max = 0;
    limity_min = 0;
    limity_max = 0;
    limitz_min = 0;
    limitz_max = 0;
    frame_width = 640;
    frame_height = 480;

    //load arguments
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "help")
        ("imgtype,t", po::value<std::string>(&img_type)->default_value(".png"), "image file type")
        ("initial,i", po::value<int>(&initial_frame)->default_value(0), "Initial number of the frame")
        ("padding,p", po::value<int>(&padding)->default_value(3), "Pad the number with 0 to a set amount of digits")
        ("ascii,a", po::bool_switch(&binary_mode)->default_value(true), "Save pcd files in ascii mode")
        ("oni", po::bool_switch(&no_oni)->default_value(true), "Save .oni record.")
        ("only-depth,d", po::bool_switch(&only_depth)->default_value(false), "Only process the depth stream.")
    ;
    //change the binary mode depending on the ascii value
//    binary_mode = !binary_mode;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if( vm.count("help") || vm.count("h") ) {
        std::cout << desc << "\n";
        exit(0);
    }

    // initialize device and sensors
    rc = openni::OpenNI::initialize();
    if (rc != openni::STATUS_OK)
        error_manager(1);


    rc = device.open(openni::ANY_DEVICE);
    if (rc != openni::STATUS_OK)
        error_manager(2);


    rc = depth.create(device, openni::SENSOR_DEPTH);
    if (rc != openni::STATUS_OK)
        error_manager(3);

    // set the new resolution and fps
    openni::VideoMode depth_videoMode  = depth.getVideoMode();
    depth_videoMode.setResolution(frame_width,frame_height);
    depth_videoMode.setFps(30);
    depth.setVideoMode(depth_videoMode);

    rc = depth.start();
    if (rc != openni::STATUS_OK)
        error_manager(4, true);
        // try to read the color stream
        const openni::SensorInfo* tmp= device.getSensorInfo(openni::SENSOR_COLOR);
        if (tmp != NULL){
            rc = color.create(device, openni::SENSOR_COLOR);
            if (rc != openni::STATUS_OK){
                error_manager(5, true);
            }
            if (!only_depth){
                // set the new resolution and fps
                openni::VideoMode color_videoMode  = color.getVideoMode();
                color_videoMode.setResolution(frame_width,frame_height);
                color_videoMode.setFps(30);
                color.setVideoMode(color_videoMode);

                rc = color.start();
                if (rc != openni::STATUS_OK)
                    error_manager(6, true);

                // align the depth and color image
                device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR );
            }
        } else {
            only_depth = true;
        }

    std::cout << "Sensors initialized" << std::endl;
}

void Viewer::close_all(){
    std::cout << "Stopping sensors" << std::endl;
    depth.stop();
    depth.destroy();
    if (!only_depth){
        color.stop();
        color.destroy();
    }
    device.close();
    recorder.stop();
    recorder.destroy();
    //QFile::remove("recording.oni");
    openni::OpenNI::shutdown();
    std::cout << "Sensors stopped" << std::endl;
}

Viewer::~Viewer(){
    close_all();
}
