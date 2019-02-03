
#include <iostream>
#include <linux/videodev2.h>
//FFMPEG LIBRARIES
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"

#include "libavdevice/avdevice.h"

#include "libavfilter/avfilter.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"

#include "libavformat/avformat.h"
#include "libavformat/avio.h"

    // lib swresample

#include "libswscale/swscale.h"

#include "libavutil/imgutils.h"
}

int WriteJPEG(AVCodecContext *pCodecCtx, AVFrame *pFrame, int FrameNo);

int main(int argc, char const *argv[])
{
    int videoStreamIndex = -1;
    int unscaled_width = 1280;
    int unscaled_height = 720;
    int scaled_width = 1280;
    int scaled_height = 720;
    int got_picture = 0;

    /*Input Source Variables*/
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary *dicts = NULL;

    /* decoder variables */
    AVCodec *dec_codec;
    AVCodecContext *dec_context;
    AVFrame *dec_frame;
    AVPacket dec_pkt;
    AVPixelFormat opt_pxfmt = PIX_FMT_RGB24;

    /* conversion variables */
    SwsContext *conversionContext;

    /* scale variables */
    AVFrame *scaled_frame;
    SwsContext *scaleContext;

    /*Initiate all the Setups*/
    av_register_all();
    avcodec_register_all();
    avdevice_register_all();
    avformat_network_init();

    int rc = av_dict_set(&dicts, "rtsp_transport", "tcp", 0); // default udp. Set tcp interleaved mode
    if (rc < 0)
    {
        return EXIT_FAILURE;
    }

    if (avformat_open_input(&pFormatCtx, "rtsp://admin:planysch4pn@192.168.0.125:554", NULL, &dicts) != 0)
    {
        return EXIT_FAILURE;
    }
    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        return -1; // Couldn't find stream information
    }

    for (int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            std::cout << "Found Stream Index: " << videoStreamIndex << std::endl;
        }
        // if (input_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        //     aidx = i;
    }
    if (videoStreamIndex == -1)
    {
        return -1; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    dec_context = pFormatCtx->streams[videoStreamIndex]->codec;

    /*
     * Initialize decoder
     */
    dec_codec = avcodec_find_decoder(dec_context->codec_id);
    if (!dec_codec)
    {
        return 0;
    }

    dec_context = avcodec_alloc_context3(dec_codec);
    if (!dec_context)
    {
        return 0;
    }

    avcodec_get_context_defaults3(dec_context, dec_codec);
    /*
  avcodec_get_context_defaults3 :
  Set the fields of the given AVCodecContext to default values corresponding to
  the given codec (defaults may be codec-dependent).
  */

    dec_context->width = unscaled_width;
    dec_context->height = unscaled_height;
    dec_context->pix_fmt = opt_pxfmt;

    if (avcodec_open2(dec_context, dec_codec, NULL) < 0)
    {
        return 0;
    }
    dec_frame = av_frame_alloc();

    if (!dec_frame)
    {
        return 0;
    }

    if (av_image_alloc(dec_frame->data, dec_frame->linesize, dec_context->width,
                       dec_context->height, dec_context->pix_fmt, 32) < 0)
    {
        return 0;
    }

    /*
   * Allocate frame data
   */
    scaled_frame = av_frame_alloc();

    if (!scaled_frame)
    {
        return 0;
    }

    int scaled_size =
        av_image_get_buffer_size(dec_context->pix_fmt, scaled_width, scaled_height, 1);

    uint8_t *video_outbuf = (uint8_t *)av_malloc(scaled_size);

    int retValue =
        av_image_fill_arrays(scaled_frame->data, scaled_frame->linesize,
                             video_outbuf, opt_pxfmt, scaled_height, scaled_width,
                             1);

    av_init_packet(&dec_pkt);

    /* Iterator to Certain Number of Images */
    int noOfImagesToSave = 0;

    while (av_read_frame(pFormatCtx, &dec_pkt) >= 0)
    {
        // std::cout << "Finished Reading Frame" << std::endl;
        // Is this a packet from the video stream?
        if (dec_pkt.stream_index == videoStreamIndex)
        {
            // std::cout << "Frame index matches Video Index" << std::endl;

            if (avcodec_decode_video2(dec_context, scaled_frame, &got_picture, &dec_pkt))
            {
                if (got_picture)
                {
                    // std::cout << "Got Picture" << std::endl;

                    dec_frame->width = unscaled_width;
                    dec_frame->height = unscaled_height;
                    dec_frame->format = opt_pxfmt;
                    conversionContext =
                        sws_getContext(unscaled_width, unscaled_height, opt_pxfmt,
                                       unscaled_width, unscaled_height, AV_PIX_FMT_RGB24,
                                       SWS_FAST_BILINEAR, NULL, NULL, NULL);

                    sws_scale(conversionContext, dec_frame->data, dec_frame->linesize, 0,
                              unscaled_height, scaled_frame->data, scaled_frame->linesize);

                    if (noOfImagesToSave++ < 1)
                    {
                        WriteJPEG(dec_context, scaled_frame, noOfImagesToSave);
                    }
                }
            }
        }
        else
        {
            // std::cout << "[Debug][Read Frame] Video Index Mismtach" << std::endl;
        }
    }

    return 0;
}

int WriteJPEG(AVCodecContext *pCodecCtx, AVFrame *pFrame, int FrameNo)
{
    std::cout << "Wrting to JPG Images" << std::endl;
    AVCodecContext *pOCodecCtx;
    AVCodec *pOCodec;
    uint8_t *Buffer;
    int BufSiz;
    int BufSizActual;
    int ImgFmt = AV_PIX_FMT_YUVJ420P; //for the newer ffmpeg version, this int to pixelformat
    FILE *JPEGFile;
    char JPEGFName[256];

    BufSiz = avpicture_get_size(
        PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

    Buffer = (uint8_t *)malloc(BufSiz);
    if (Buffer == NULL)
        return (0);
    memset(Buffer, 0, BufSiz);
    AVCodec *codecEncode = avcodec_find_encoder(AV_CODEC_ID_MJPEG);

    pOCodecCtx = avcodec_alloc_context3(codecEncode);
    if (!pOCodecCtx)
    {
        free(Buffer);
        return (0);
    }

    pOCodecCtx->bit_rate = pCodecCtx->bit_rate;
    pOCodecCtx->width = pCodecCtx->width;
    pOCodecCtx->height = pCodecCtx->height;
    pOCodecCtx->pix_fmt = (AVPixelFormat)ImgFmt;
    pOCodecCtx->codec_id = AV_CODEC_ID_MJPEG;
    pOCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pOCodecCtx->time_base.num = pCodecCtx->time_base.num;
    pOCodecCtx->time_base.den = pCodecCtx->time_base.den;

    pOCodec = avcodec_find_encoder(pOCodecCtx->codec_id);
    if (!pOCodec)
    {
        free(Buffer);
        return (0);
    }
    if (avcodec_open2(pOCodecCtx, pOCodec, NULL) < 0)
    {
        free(Buffer);
        return (0);
    }

    pOCodecCtx->mb_lmin = pOCodecCtx->lmin =
        pOCodecCtx->qmin * FF_QP2LAMBDA;
    pOCodecCtx->mb_lmax = pOCodecCtx->lmax =
        pOCodecCtx->qmax * FF_QP2LAMBDA;
    pOCodecCtx->flags = CODEC_FLAG_QSCALE;
    pOCodecCtx->global_quality = pOCodecCtx->qmin * FF_QP2LAMBDA;

    pFrame->pts = 1;
    pFrame->quality = pOCodecCtx->global_quality;
    BufSizActual = avcodec_encode_video(
        pOCodecCtx, Buffer, BufSiz, pFrame);
    // char bufferChar = (char)Buffer;

    // std::string testString;
    // for (int i = 0; i < BufSizActual; i++)
    // {
    //     const char testChar = (const char)(*(Buffer + i));
    //     testString += testChar;
    // }
    // std::cout << "Buffer: " << testString << std::endl;

    sprintf(JPEGFName, "%06d.jpg", FrameNo);
    JPEGFile = fopen(JPEGFName, "wb");
    fwrite(Buffer, 1, BufSizActual, JPEGFile);
    fclose(JPEGFile);

    avcodec_close(pOCodecCtx);
    free(Buffer);
    return (BufSizActual);
}
