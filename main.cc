#include <math.h>
#include <iostream>
extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
}

AVFormatContext *_format_context = NULL;

bool open_file(const char *file_path) {
    AVDictionary *dictionary = NULL;
    av_dict_set(&dictionary, "protocol_whitelist", "file,udp,rtp", 0);
    av_dict_set(&dictionary, "probesize", "32", 0);

    int ret = avformat_open_input(&_format_context, file_path, NULL, &dictionary);
    if (ret != 0) {
        std::cout << "Open input failed" << std::endl;
        return false;
    }
    return true;
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        std::cout << "please provide the input file" << std::endl;
        return 1;
    }

    if (!open_file(argv[1])) {
        return 1;
    }
    return 0;
}
