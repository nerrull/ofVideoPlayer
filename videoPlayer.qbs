import qbs
import qbs.Process
import qbs.File
import qbs.FileInfo
import qbs.TextFile
import "../../../libs/openFrameworksCompiled/project/qtcreator/ofApp.qbs" as ofApp

Project{
    property string of_root: "../../.."

    ofApp {
        name: { return FileInfo.baseName(path) }

        files: [
            "src/audiosampleplayer.cpp",
            "src/audiosampleplayer.h",
            "src/happlayermanager.cpp",
            "src/happlayermanager.h",
            "src/imageloader.cpp",
            "src/imageloader.h",
            "src/simplesampleplayer.cpp",
            "src/simplesampleplayer.h",
            'src/main.cpp',
            'src/ofApp.cpp',
            'src/ofApp.h',
            'src/ofxThreadedVideoPlayerManager.cpp',
            'src/ofxThreadedVideoPlayerManager.h',
            "bin/data/settings.json"

        ]

        of.addons: [
            'ofxOsc',
            'ofxGaplessVideo',
            'ofxHDF5',
            'ofxHapPlayer',
            'ofxSoundObjects',
            'ofxJSON',
            'ofxJsonSettings'
        ]

        // additional flags for the project. the of module sets some
        // flags by default to add the core libraries, search paths...
        // this flags can be augmented through the following properties:
        of.pkgConfigs: []       // list of additional system pkgs to include
        of.includePaths: []     // include search paths
        of.cFlags: []           // flags passed to the c compiler
        of.cxxFlags: []         // flags passed to the c++ compiler
        of.linkerFlags: []      // flags passed to the linker
        of.defines: []          // defines are passed as -D to the compiler
                                // and can be checked with #ifdef or #if in the code

        // other flags can be set through the cpp module: http://doc.qt.io/qbs/cpp-module.html
        // eg: this will enable ccache when compiling
        //
        // cpp.compilerWrapper: 'ccache'

        Depends{
            name: "cpp"
        }

        // common rules that parse the include search paths, core libraries...
        Depends{
            name: "of"
        }

        // dependency with the OF library
        Depends{
            name: "openFrameworks"
        }
    }

    references: [FileInfo.joinPaths(of_root, "/libs/openFrameworksCompiled/project/qtcreator/openFrameworks.qbs")]
}
