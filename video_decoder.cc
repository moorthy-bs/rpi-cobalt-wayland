//
// If not stated otherwise in this file or this component's LICENSE file the
// following copyright and licenses apply:
//
// Copyright 2017 Arris
// Copyright 2019 RDK Management
// Copyright 2019 Liberty Global B.V.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "third_party/starboard/raspi/wayland/video_decoder.h"

#include "starboard/shared/starboard/media/media_support_internal.h"

#include <gst/gst.h>

namespace
{

const char* GetVideoCodecName(SbMediaVideoCodec codec)
{
  const char* name;
  switch (codec) {
#define VIDEO_CASE(k) case k: name = #k; break;
  VIDEO_CASE(kSbMediaVideoCodecNone)
  VIDEO_CASE(kSbMediaVideoCodecH264)
  VIDEO_CASE(kSbMediaVideoCodecMpeg2)
  VIDEO_CASE(kSbMediaVideoCodecTheora)
  VIDEO_CASE(kSbMediaVideoCodecVc1)
  VIDEO_CASE(kSbMediaVideoCodecVp8)
#undef VIDEO_CASE
  default:
    name = "UNKNOWN";
  }
  return name;
}
}

SB_EXPORT bool SbMediaIsVideoSupported(SbMediaVideoCodec videoCodec,
					int profile,
                                       int level,
                                       int bit_depth,
                                       SbMediaPrimaryId primary_id,
                                       SbMediaTransferId transfer_id,
                                       SbMediaMatrixId matrix_id,
                                       int frameWidth,
                                       int frameHeight,
                                       int64_t bitrate,
                                       int fps
#if SB_API_VERSION >= 10
                                       ,
                                       bool decode_to_texture_required
#endif  // SB_API_VERSION >= 10
                                       )
{
  const bool result = (videoCodec == kSbMediaVideoCodecH264
                       || videoCodec == kSbMediaVideoCodecMpeg2
			|| videoCodec == kSbMediaVideoCodecVc1
                       || videoCodec == kSbMediaVideoCodecVp8)
    && frameWidth >= 0 && frameWidth <= 5000
    && frameHeight >= 0 && frameHeight <= 5000
    && bitrate >= 0 && bitrate <= SB_MEDIA_MAX_VIDEO_BITRATE_IN_BITS_PER_SECOND
    && fps >= 0 && fps <= 60;
  return result;
}

VideoDecoder::VideoDecoder(SbPlayerPrivate& player)
  : AbstractDecoder(player, kSbMediaTypeVideo)
{
}

GstCaps* VideoDecoder::CustomInitialize()
{
  g_object_set(Source, "min-percent", 60u, "max-bytes", 256ull << 10, NULL);

  const char* type = nullptr;
  GstCaps* caps = nullptr;
  switch (Player.GetVideoCodec()) {
  case kSbMediaVideoCodecH264:
    caps = gst_caps_new_simple("video/x-h264",
                               "stream-format", G_TYPE_STRING, "byte-stream",
                               nullptr);
    break;
  case kSbMediaVideoCodecMpeg2:
    type = "video/x-mpeg";
    break;
  case kSbMediaVideoCodecVc1:
    type =  "video/x-vc1";
    break;
  case kSbMediaVideoCodecVp8:
    type =  "video/x-vp8";
    break;
  default:
    SB_LOG(INFO) << "Unsupported video codec "
      << GetVideoCodecName(Player.GetVideoCodec());
    return nullptr;
  }
  if (!caps) {
    caps = gst_caps_new_empty_simple(type);
  }

#if 0
  gchar* c = gst_caps_to_string(caps);
  SB_DLOG(INFO) << "video caps are '" << c << "'";
  g_free(c);
#endif

  return caps;
}

