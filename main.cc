#include <math.h>
#include <iostream>
#include <chrono>
extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
}

using namespace std::chrono;

AVFormatContext *_format_context = NULL;
int _width = -1;
int _height = -1;

bool open_file(const char *file_path) {
    AVDictionary *dictionary = NULL;
    av_dict_set(&dictionary, "protocol_whitelist", "file,udp,rtp", 0);
    av_dict_set(&dictionary, "probesize", "32", 0);

    int ret = avformat_open_input(&_format_context, file_path, NULL, &dictionary);
    if (ret != 0) {
        std::cout << "Open input failed" << std::endl;
        return false;
    }

    _format_context->flags = AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;
    avformat_find_stream_info(_format_context, NULL);
   
 ///< build stream info
    for (uint32_t i = 0; i < _format_context->nb_streams; i++) {
        AVStream *av_stream = _format_context->streams[i];
        AVCodecParameters *codec_param = av_stream->codecpar;
        if (codec_param == NULL) {
            continue;
        }
        else if (codec_param->codec_type == AVMEDIA_TYPE_VIDEO) {
            std::cout << "New video stream : " << std::endl;
            _width = codec_param->width;
            _height = codec_param->height;
            std::cout << "\t width:" << codec_param->width << std::endl;
            std::cout << "\t height:" << codec_param->height << std::endl;
        }
    }    
    std::cout << "Open input file " << file_path << " success!" << std::endl;
    return true;
}

const AVCodec *_avcodec;
AVCodecContext *_avcodec_context;

bool open_codec() {
    enum AVCodecID ff_codec_id = AV_CODEC_ID_H264;
    _avcodec = avcodec_find_decoder(ff_codec_id);
    if (_avcodec == NULL) {
        std::cout << "Cannot find decoder" << std::endl;
        return false;
    }
    _avcodec_context = avcodec_alloc_context3(_avcodec);
    _avcodec_context->extradata = NULL;
    _avcodec_context->extradata_size = 0;
    _avcodec_context->width = _width;
    _avcodec_context->height = _height;
    uint32_t num_threads = 1;
    _avcodec_context->thread_type = FF_THREAD_FRAME;
    _avcodec_context->thread_count = num_threads;
    
    AVDictionary *opts = NULL;
    av_dict_set(&opts, "flags", "low_delay", 0);
    int ret = avcodec_open2(_avcodec_context, _avcodec, &opts);
    if (ret < 0) {
        std::cout << "Cannot open decoder" << std::endl;
        return false;
    }
    
    std::cout << "Open decoder success !" << std::endl;
    return true;
}

bool _eof = false;

bool read_and_decode() {
    AVPacket package;
    do {
        int ret = av_read_frame(_format_context, &package);
        if (ret < 0) {
            if (AVERROR(EAGAIN) == ret) {
                continue;
            } else {
                _eof = true;
                break;
            }
        }
        
        const uint64_t presentationTimeUS = (int64_t)duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
        package.pts = presentationTimeUS;

        AVFrame *decoded_frame = av_frame_alloc();
        ret = avcodec_send_packet(_avcodec_context, &package);
        if (ret != 0) {
            std::cout << "send video packet failed";
            av_frame_unref(decoded_frame);
            break;
        }
        ret = avcodec_receive_frame(_avcodec_context, decoded_frame);
        if (ret != 0) {
            if (ret == AVERROR(EAGAIN)) {
                av_frame_unref(decoded_frame);
                std::cout << "decoder cause eagain" << std::endl;
                continue;
            } else {
                std::cout << "cannot decoder video, " << ret << std::endl;
            }
            av_frame_unref(decoded_frame);
            break;
        }

        const auto now=steady_clock::now();
        const int64_t nowUS=(int64_t)duration_cast<microseconds>(now.time_since_epoch()).count();
        std::cout << "Got new frame:" << nowUS - decoded_frame->pts << "us" <<std::endl;
        static int save_to_file = true;
        if (save_to_file) {
            save_to_file = false;
            FILE *fp = fopen("out.yuv", "wb");
            if (fp == NULL) {
                std::cout << "cannot open file" << std::endl;
            }
            if (decoded_frame->linesize[0] > 0) {
                fwrite(decoded_frame->data[0], decoded_frame->linesize[0] * decoded_frame->height, 1, fp);
            }
            if (decoded_frame->linesize[1] > 0) {
                fwrite(decoded_frame->data[1], decoded_frame->linesize[1] * decoded_frame->height/2, 1, fp);
            }
            if (decoded_frame->linesize[2] > 0) {
                fwrite(decoded_frame->data[2], decoded_frame->linesize[2] * decoded_frame->height/2, 1, fp);
            }
            fclose(fp);
        }
        //std::cout << "linesize " << decoded_frame->linesize[0] << std::endl;
        av_frame_unref(decoded_frame);
    } while (true);
    std::cout << "read end of file" << std::endl;
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

    if (!open_codec()) {
        return 1;
    }

    read_and_decode();

    return 0;
}
