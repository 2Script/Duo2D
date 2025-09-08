#pragma once
#include <climits>
#include <array>
#include <cstdint>
#include <frozen/unordered_map.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>


namespace d2d::vk::impl {
    constexpr std::size_t num_pixel_formats = (VK_FORMAT_ASTC_12x12_SRGB_BLOCK + 1) + 34 + 4 + 2 + 14 + 8 + 1 + 2;
}


namespace d2d::vk {
    struct pixel_format_info {

        struct channel { enum name {
            red, green, blue, alpha, depth, stencil, exponent, num_channels
        };};
    public:
        VkFormat id;

        bool packed;
        std::array<std::uint_fast8_t, channel::num_channels> channel_size_bits;
        std::uint_fast8_t total_size_bytes;
        enum integer_format : std::uint_fast8_t {
            unsigned_normalized, signed_normalized, 
            unsigned_scaled,     signed_scaled, 
            unsigned_integer,    signed_integer,
            unsigned_float,      signed_float,
            unsigned_normalized_encoded,

            num_integer_formats
        } format;
        enum compression_method : std::uint_fast8_t {
            none, S3TC, ETC, ASTC, PVRTC
        } compression = none;
        enum image_aspect : std::uint_least8_t {//VkImageAspectFlags {
            color = VK_IMAGE_ASPECT_COLOR_BIT,
            plane2 = VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT,
            plane3 = VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT,
        } aspect = color; //TODO

        //constexpr std::vector<channel::name> order() const noexcept;
    };
}



namespace d2d::vk {
    constexpr frozen::unordered_map<VkFormat, pixel_format_info, impl::num_pixel_formats> pixel_formats{
        {VK_FORMAT_UNDEFINED, pixel_format_info{}},

        
        {VK_FORMAT_R4G4_UNORM_PACK8, {VK_FORMAT_R4G4_UNORM_PACK8, true, {4,4,0,0}, 8 / CHAR_BIT, pixel_format_info::unsigned_normalized}},

        {VK_FORMAT_R4G4B4A4_UNORM_PACK16, {VK_FORMAT_R4G4B4A4_UNORM_PACK16, true, {4,4,4,4}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_B4G4R4A4_UNORM_PACK16, {VK_FORMAT_B4G4R4A4_UNORM_PACK16, true, {4,4,4,4}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        
        {VK_FORMAT_R5G6B5_UNORM_PACK16, {VK_FORMAT_R5G6B5_UNORM_PACK16, true, {5,6,5}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_B5G6R5_UNORM_PACK16, {VK_FORMAT_B5G6R5_UNORM_PACK16, true, {5,6,5}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        
        {VK_FORMAT_R5G5B5A1_UNORM_PACK16, {VK_FORMAT_R5G5B5A1_UNORM_PACK16, true, {5,5,5,1}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_B5G5R5A1_UNORM_PACK16, {VK_FORMAT_B5G5R5A1_UNORM_PACK16, true, {5,5,5,1}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_A1R5G5B5_UNORM_PACK16, {VK_FORMAT_A1R5G5B5_UNORM_PACK16, true, {5,5,5,1}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        

        {VK_FORMAT_R8_UNORM,   {VK_FORMAT_R8_UNORM,   false, {8}, 8 / CHAR_BIT, pixel_format_info::unsigned_normalized        }},
        {VK_FORMAT_R8_SNORM,   {VK_FORMAT_R8_SNORM,   false, {8}, 8 / CHAR_BIT, pixel_format_info::signed_normalized          }},
        {VK_FORMAT_R8_USCALED, {VK_FORMAT_R8_USCALED, false, {8}, 8 / CHAR_BIT, pixel_format_info::unsigned_scaled            }},
        {VK_FORMAT_R8_SSCALED, {VK_FORMAT_R8_SSCALED, false, {8}, 8 / CHAR_BIT, pixel_format_info::signed_scaled              }},
        {VK_FORMAT_R8_UINT,    {VK_FORMAT_R8_UINT,    false, {8}, 8 / CHAR_BIT, pixel_format_info::unsigned_integer           }},
        {VK_FORMAT_R8_SINT,    {VK_FORMAT_R8_SINT,    false, {8}, 8 / CHAR_BIT, pixel_format_info::signed_integer             }},
        {VK_FORMAT_R8_SRGB,    {VK_FORMAT_R8_SRGB,    false, {8}, 8 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded}},

        {VK_FORMAT_R8G8_UNORM,   {VK_FORMAT_R8G8_UNORM,   false, {8, 8}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized        }},
        {VK_FORMAT_R8G8_SNORM,   {VK_FORMAT_R8G8_SNORM,   false, {8, 8}, 16 / CHAR_BIT, pixel_format_info::signed_normalized          }},
        {VK_FORMAT_R8G8_USCALED, {VK_FORMAT_R8G8_USCALED, false, {8, 8}, 16 / CHAR_BIT, pixel_format_info::unsigned_scaled            }},
        {VK_FORMAT_R8G8_SSCALED, {VK_FORMAT_R8G8_SSCALED, false, {8, 8}, 16 / CHAR_BIT, pixel_format_info::signed_scaled              }},
        {VK_FORMAT_R8G8_UINT,    {VK_FORMAT_R8G8_UINT,    false, {8, 8}, 16 / CHAR_BIT, pixel_format_info::unsigned_integer           }},
        {VK_FORMAT_R8G8_SINT,    {VK_FORMAT_R8G8_SINT,    false, {8, 8}, 16 / CHAR_BIT, pixel_format_info::signed_integer             }},
        {VK_FORMAT_R8G8_SRGB,    {VK_FORMAT_R8G8_SRGB,    false, {8, 8}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded}},
        
        {VK_FORMAT_R8G8B8_UNORM,   {VK_FORMAT_R8G8B8_UNORM,   false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_normalized        }},
        {VK_FORMAT_R8G8B8_SNORM,   {VK_FORMAT_R8G8B8_SNORM,   false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::signed_normalized          }},
        {VK_FORMAT_R8G8B8_USCALED, {VK_FORMAT_R8G8B8_USCALED, false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_scaled            }},
        {VK_FORMAT_R8G8B8_SSCALED, {VK_FORMAT_R8G8B8_SSCALED, false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::signed_scaled              }},
        {VK_FORMAT_R8G8B8_UINT,    {VK_FORMAT_R8G8B8_UINT,    false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_integer           }},
        {VK_FORMAT_R8G8B8_SINT,    {VK_FORMAT_R8G8B8_SINT,    false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::signed_integer             }},
        {VK_FORMAT_R8G8B8_SRGB,    {VK_FORMAT_R8G8B8_SRGB,    false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded}},
        {VK_FORMAT_B8G8R8_UNORM,   {VK_FORMAT_B8G8R8_UNORM,   false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_normalized        }},
        {VK_FORMAT_B8G8R8_SNORM,   {VK_FORMAT_B8G8R8_SNORM,   false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::signed_normalized          }},
        {VK_FORMAT_B8G8R8_USCALED, {VK_FORMAT_B8G8R8_USCALED, false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_scaled            }},
        {VK_FORMAT_B8G8R8_SSCALED, {VK_FORMAT_B8G8R8_SSCALED, false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::signed_scaled              }},
        {VK_FORMAT_B8G8R8_UINT,    {VK_FORMAT_B8G8R8_UINT,    false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_integer           }},
        {VK_FORMAT_B8G8R8_SINT,    {VK_FORMAT_B8G8R8_SINT,    false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::signed_integer             }},
        {VK_FORMAT_B8G8R8_SRGB,    {VK_FORMAT_B8G8R8_SRGB,    false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded}},
        
        {VK_FORMAT_R8G8B8A8_UNORM,   {VK_FORMAT_R8G8B8A8_UNORM,   false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized        }},
        {VK_FORMAT_R8G8B8A8_SNORM,   {VK_FORMAT_R8G8B8A8_SNORM,   false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::signed_normalized          }},
        {VK_FORMAT_R8G8B8A8_USCALED, {VK_FORMAT_R8G8B8A8_USCALED, false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_scaled            }},
        {VK_FORMAT_R8G8B8A8_SSCALED, {VK_FORMAT_R8G8B8A8_SSCALED, false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::signed_scaled              }},
        {VK_FORMAT_R8G8B8A8_UINT,    {VK_FORMAT_R8G8B8A8_UINT,    false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_integer           }},
        {VK_FORMAT_R8G8B8A8_SINT,    {VK_FORMAT_R8G8B8A8_SINT,    false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::signed_integer             }},
        {VK_FORMAT_R8G8B8A8_SRGB,    {VK_FORMAT_R8G8B8A8_SRGB,    false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded}},
        {VK_FORMAT_B8G8R8A8_UNORM,   {VK_FORMAT_B8G8R8A8_UNORM,   false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized        }},
        {VK_FORMAT_B8G8R8A8_SNORM,   {VK_FORMAT_B8G8R8A8_SNORM,   false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::signed_normalized          }},
        {VK_FORMAT_B8G8R8A8_USCALED, {VK_FORMAT_B8G8R8A8_USCALED, false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_scaled            }},
        {VK_FORMAT_B8G8R8A8_SSCALED, {VK_FORMAT_B8G8R8A8_SSCALED, false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::signed_scaled              }},
        {VK_FORMAT_B8G8R8A8_UINT,    {VK_FORMAT_B8G8R8A8_UINT,    false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_integer           }},
        {VK_FORMAT_B8G8R8A8_SINT,    {VK_FORMAT_B8G8R8A8_SINT,    false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::signed_integer             }},
        {VK_FORMAT_B8G8R8A8_SRGB,    {VK_FORMAT_B8G8R8A8_SRGB,    false, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded}},
        
        {VK_FORMAT_A8B8G8R8_UNORM_PACK32,   {VK_FORMAT_A8B8G8R8_UNORM_PACK32,   true, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized        }},
        {VK_FORMAT_A8B8G8R8_SNORM_PACK32,   {VK_FORMAT_A8B8G8R8_SNORM_PACK32,   true, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::signed_normalized          }},
        {VK_FORMAT_A8B8G8R8_USCALED_PACK32, {VK_FORMAT_A8B8G8R8_USCALED_PACK32, true, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_scaled            }},
        {VK_FORMAT_A8B8G8R8_SSCALED_PACK32, {VK_FORMAT_A8B8G8R8_SSCALED_PACK32, true, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::signed_scaled              }},
        {VK_FORMAT_A8B8G8R8_UINT_PACK32,    {VK_FORMAT_A8B8G8R8_UINT_PACK32,    true, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_integer           }},
        {VK_FORMAT_A8B8G8R8_SINT_PACK32,    {VK_FORMAT_A8B8G8R8_SINT_PACK32,    true, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::signed_integer             }},
        {VK_FORMAT_A8B8G8R8_SRGB_PACK32,    {VK_FORMAT_A8B8G8R8_SRGB_PACK32,    true, {8,8,8,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded}},
        

        {VK_FORMAT_A2R10G10B10_UNORM_PACK32,   {VK_FORMAT_A2R10G10B10_UNORM_PACK32,   true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_A2R10G10B10_SNORM_PACK32,   {VK_FORMAT_A2R10G10B10_SNORM_PACK32,   true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::signed_normalized  }},
        {VK_FORMAT_A2R10G10B10_USCALED_PACK32, {VK_FORMAT_A2R10G10B10_USCALED_PACK32, true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::unsigned_scaled    }},
        {VK_FORMAT_A2R10G10B10_SSCALED_PACK32, {VK_FORMAT_A2R10G10B10_SSCALED_PACK32, true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::signed_scaled      }},
        {VK_FORMAT_A2R10G10B10_UINT_PACK32,    {VK_FORMAT_A2R10G10B10_UINT_PACK32,    true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::unsigned_integer   }},
        {VK_FORMAT_A2R10G10B10_SINT_PACK32,    {VK_FORMAT_A2R10G10B10_SINT_PACK32,    true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::signed_integer     }},
        {VK_FORMAT_A2B10G10R10_UNORM_PACK32,   {VK_FORMAT_A2B10G10R10_UNORM_PACK32,   true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_A2B10G10R10_SNORM_PACK32,   {VK_FORMAT_A2B10G10R10_SNORM_PACK32,   true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::signed_normalized  }},
        {VK_FORMAT_A2B10G10R10_USCALED_PACK32, {VK_FORMAT_A2B10G10R10_USCALED_PACK32, true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::unsigned_scaled    }},
        {VK_FORMAT_A2B10G10R10_SSCALED_PACK32, {VK_FORMAT_A2B10G10R10_SSCALED_PACK32, true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::signed_scaled      }},
        {VK_FORMAT_A2B10G10R10_UINT_PACK32,    {VK_FORMAT_A2B10G10R10_UINT_PACK32,    true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::unsigned_integer   }},
        {VK_FORMAT_A2B10G10R10_SINT_PACK32,    {VK_FORMAT_A2B10G10R10_SINT_PACK32,    true, {10,10,10,2}, 32 / CHAR_BIT, pixel_format_info::signed_integer     }},
        

        {VK_FORMAT_R16_UNORM,   {VK_FORMAT_R16_UNORM,   false, {16}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_R16_SNORM,   {VK_FORMAT_R16_SNORM,   false, {16}, 16 / CHAR_BIT, pixel_format_info::signed_normalized  }},
        {VK_FORMAT_R16_USCALED, {VK_FORMAT_R16_USCALED, false, {16}, 16 / CHAR_BIT, pixel_format_info::unsigned_scaled    }},
        {VK_FORMAT_R16_SSCALED, {VK_FORMAT_R16_SSCALED, false, {16}, 16 / CHAR_BIT, pixel_format_info::signed_scaled      }},
        {VK_FORMAT_R16_UINT,    {VK_FORMAT_R16_UINT,    false, {16}, 16 / CHAR_BIT, pixel_format_info::unsigned_integer   }},
        {VK_FORMAT_R16_SINT,    {VK_FORMAT_R16_SINT,    false, {16}, 16 / CHAR_BIT, pixel_format_info::signed_integer     }},
        {VK_FORMAT_R16_SFLOAT,  {VK_FORMAT_R16_SFLOAT,  false, {16}, 16 / CHAR_BIT, pixel_format_info::signed_float       }},
        
        {VK_FORMAT_R16G16_UNORM,   {VK_FORMAT_R16G16_UNORM,   false, {16,16}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_R16G16_SNORM,   {VK_FORMAT_R16G16_SNORM,   false, {16,16}, 32 / CHAR_BIT, pixel_format_info::signed_normalized  }},
        {VK_FORMAT_R16G16_USCALED, {VK_FORMAT_R16G16_USCALED, false, {16,16}, 32 / CHAR_BIT, pixel_format_info::unsigned_scaled    }},
        {VK_FORMAT_R16G16_SSCALED, {VK_FORMAT_R16G16_SSCALED, false, {16,16}, 32 / CHAR_BIT, pixel_format_info::signed_scaled      }},
        {VK_FORMAT_R16G16_UINT,    {VK_FORMAT_R16G16_UINT,    false, {16,16}, 32 / CHAR_BIT, pixel_format_info::unsigned_integer   }},
        {VK_FORMAT_R16G16_SINT,    {VK_FORMAT_R16G16_SINT,    false, {16,16}, 32 / CHAR_BIT, pixel_format_info::signed_integer     }},
        {VK_FORMAT_R16G16_SFLOAT,  {VK_FORMAT_R16G16_SFLOAT,  false, {16,16}, 32 / CHAR_BIT, pixel_format_info::signed_float       }},
        
        {VK_FORMAT_R16G16B16_UNORM,   {VK_FORMAT_R16G16B16_UNORM,   false, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_R16G16B16_SNORM,   {VK_FORMAT_R16G16B16_SNORM,   false, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::signed_normalized  }},
        {VK_FORMAT_R16G16B16_USCALED, {VK_FORMAT_R16G16B16_USCALED, false, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::unsigned_scaled    }},
        {VK_FORMAT_R16G16B16_SSCALED, {VK_FORMAT_R16G16B16_SSCALED, false, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::signed_scaled      }},
        {VK_FORMAT_R16G16B16_UINT,    {VK_FORMAT_R16G16B16_UINT,    false, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::unsigned_integer   }},
        {VK_FORMAT_R16G16B16_SINT,    {VK_FORMAT_R16G16B16_SINT,    false, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::signed_integer     }},
        {VK_FORMAT_R16G16B16_SFLOAT,  {VK_FORMAT_R16G16B16_SFLOAT,  false, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::signed_float       }},
        
        {VK_FORMAT_R16G16B16A16_UNORM,   {VK_FORMAT_R16G16B16A16_UNORM,   false, {16,16,16,16}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_R16G16B16A16_SNORM,   {VK_FORMAT_R16G16B16A16_SNORM,   false, {16,16,16,16}, 64 / CHAR_BIT, pixel_format_info::signed_normalized  }},
        {VK_FORMAT_R16G16B16A16_USCALED, {VK_FORMAT_R16G16B16A16_USCALED, false, {16,16,16,16}, 64 / CHAR_BIT, pixel_format_info::unsigned_scaled    }},
        {VK_FORMAT_R16G16B16A16_SSCALED, {VK_FORMAT_R16G16B16A16_SSCALED, false, {16,16,16,16}, 64 / CHAR_BIT, pixel_format_info::signed_scaled      }},
        {VK_FORMAT_R16G16B16A16_UINT,    {VK_FORMAT_R16G16B16A16_UINT,    false, {16,16,16,16}, 64 / CHAR_BIT, pixel_format_info::unsigned_integer   }},
        {VK_FORMAT_R16G16B16A16_SINT,    {VK_FORMAT_R16G16B16A16_SINT,    false, {16,16,16,16}, 64 / CHAR_BIT, pixel_format_info::signed_integer     }},
        {VK_FORMAT_R16G16B16A16_SFLOAT,  {VK_FORMAT_R16G16B16A16_SFLOAT,  false, {16,16,16,16}, 64 / CHAR_BIT, pixel_format_info::signed_float       }},
        

        {VK_FORMAT_R32_UINT,   {VK_FORMAT_R32_UINT,   false, {32}, 32 / CHAR_BIT, pixel_format_info::unsigned_integer}},
        {VK_FORMAT_R32_SINT,   {VK_FORMAT_R32_SINT,   false, {32}, 32 / CHAR_BIT, pixel_format_info::signed_integer  }},
        {VK_FORMAT_R32_SFLOAT, {VK_FORMAT_R32_SFLOAT, false, {32}, 32 / CHAR_BIT, pixel_format_info::signed_float    }},
        
        {VK_FORMAT_R32G32_UINT,   {VK_FORMAT_R32G32_UINT,   false, {32,32}, 64 / CHAR_BIT, pixel_format_info::unsigned_integer}},
        {VK_FORMAT_R32G32_SINT,   {VK_FORMAT_R32G32_SINT,   false, {32,32}, 64 / CHAR_BIT, pixel_format_info::signed_integer  }},
        {VK_FORMAT_R32G32_SFLOAT, {VK_FORMAT_R32G32_SFLOAT, false, {32,32}, 64 / CHAR_BIT, pixel_format_info::signed_float    }},
        
        {VK_FORMAT_R32G32B32_UINT,   {VK_FORMAT_R32G32B32_UINT,   false, {32,32,32}, 96 / CHAR_BIT, pixel_format_info::unsigned_integer}},
        {VK_FORMAT_R32G32B32_SINT,   {VK_FORMAT_R32G32B32_SINT,   false, {32,32,32}, 96 / CHAR_BIT, pixel_format_info::signed_integer  }},
        {VK_FORMAT_R32G32B32_SFLOAT, {VK_FORMAT_R32G32B32_SFLOAT, false, {32,32,32}, 96 / CHAR_BIT, pixel_format_info::signed_float    }},
        
        {VK_FORMAT_R32G32B32A32_UINT,   {VK_FORMAT_R32G32B32A32_UINT,   false, {32,32,32,32}, 128 / CHAR_BIT, pixel_format_info::unsigned_integer}},
        {VK_FORMAT_R32G32B32A32_SINT,   {VK_FORMAT_R32G32B32A32_SINT,   false, {32,32,32,32}, 128 / CHAR_BIT, pixel_format_info::signed_integer  }},
        {VK_FORMAT_R32G32B32A32_SFLOAT, {VK_FORMAT_R32G32B32A32_SFLOAT, false, {32,32,32,32}, 128 / CHAR_BIT, pixel_format_info::signed_float    }},
        
        {VK_FORMAT_R64_UINT,   {VK_FORMAT_R64_UINT,   false, {64}, 64 / CHAR_BIT, pixel_format_info::unsigned_integer}},
        {VK_FORMAT_R64_SINT,   {VK_FORMAT_R64_SINT,   false, {64}, 64 / CHAR_BIT, pixel_format_info::signed_integer  }},
        {VK_FORMAT_R64_SFLOAT, {VK_FORMAT_R64_SFLOAT, false, {64}, 64 / CHAR_BIT, pixel_format_info::signed_float    }},
        
        {VK_FORMAT_R64G64_UINT,   {VK_FORMAT_R64G64_UINT,   false, {64,64}, 128 / CHAR_BIT, pixel_format_info::unsigned_integer}},
        {VK_FORMAT_R64G64_SINT,   {VK_FORMAT_R64G64_SINT,   false, {64,64}, 128 / CHAR_BIT, pixel_format_info::signed_integer  }},
        {VK_FORMAT_R64G64_SFLOAT, {VK_FORMAT_R64G64_SFLOAT, false, {64,64}, 128 / CHAR_BIT, pixel_format_info::signed_float    }},
        
        {VK_FORMAT_R64G64B64_UINT,   {VK_FORMAT_R64G64B64_UINT,   false, {64,64,64}, 192 / CHAR_BIT, pixel_format_info::unsigned_integer}},
        {VK_FORMAT_R64G64B64_SINT,   {VK_FORMAT_R64G64B64_SINT,   false, {64,64,64}, 192 / CHAR_BIT, pixel_format_info::signed_integer  }},
        {VK_FORMAT_R64G64B64_SFLOAT, {VK_FORMAT_R64G64B64_SFLOAT, false, {64,64,64}, 192 / CHAR_BIT, pixel_format_info::signed_float    }},
        
        {VK_FORMAT_R64G64B64A64_UINT,   {VK_FORMAT_R64G64B64A64_UINT,   false, {64,64,64,64}, 256 / CHAR_BIT, pixel_format_info::unsigned_integer}},
        {VK_FORMAT_R64G64B64A64_SINT,   {VK_FORMAT_R64G64B64A64_SINT,   false, {64,64,64,64}, 256 / CHAR_BIT, pixel_format_info::signed_integer  }},
        {VK_FORMAT_R64G64B64A64_SFLOAT, {VK_FORMAT_R64G64B64A64_SFLOAT, false, {64,64,64,64}, 256 / CHAR_BIT, pixel_format_info::signed_float    }},
        

        {VK_FORMAT_B10G11R11_UFLOAT_PACK32, {VK_FORMAT_B10G11R11_UFLOAT_PACK32, true, {11,11,10}, 32 / CHAR_BIT, pixel_format_info::unsigned_float}},
        

        {VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, {VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, true, {9,9,9,0,0,0,5}, 32 / CHAR_BIT, pixel_format_info::unsigned_float}},
        

        {VK_FORMAT_D16_UNORM,           {VK_FORMAT_D16_UNORM,           false, {0,0,0,0,16},   16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_X8_D24_UNORM_PACK32, {VK_FORMAT_X8_D24_UNORM_PACK32, true,  {0,0,0,0,24},   32 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_D32_SFLOAT,          {VK_FORMAT_D32_SFLOAT,          false, {0,0,0,0,32},   32 / CHAR_BIT, pixel_format_info::signed_float       }},
        {VK_FORMAT_S8_UINT,             {VK_FORMAT_S8_UINT,             false, {0,0,0,0,0,8},  8  / CHAR_BIT, pixel_format_info::unsigned_integer   }},
        {VK_FORMAT_D16_UNORM_S8_UINT,   {VK_FORMAT_D16_UNORM_S8_UINT,   false, {0,0,0,0,16,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_D24_UNORM_S8_UINT,   {VK_FORMAT_D24_UNORM_S8_UINT,   false, {0,0,0,0,24,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_D32_SFLOAT_S8_UINT,  {VK_FORMAT_D32_SFLOAT_S8_UINT,  false, {0,0,0,0,32,8}, 64 / CHAR_BIT, pixel_format_info::signed_float       }},
        

        {VK_FORMAT_BC1_RGB_UNORM_BLOCK,  {VK_FORMAT_BC1_RGB_UNORM_BLOCK,  false, {5,6,5},    128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::S3TC}},
        {VK_FORMAT_BC1_RGB_SRGB_BLOCK,   {VK_FORMAT_BC1_RGB_SRGB_BLOCK,   false, {5,6,5},    128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::S3TC}},
        {VK_FORMAT_BC1_RGBA_UNORM_BLOCK, {VK_FORMAT_BC1_RGBA_UNORM_BLOCK, false, {5,6,5,1},  128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::S3TC}},
        {VK_FORMAT_BC1_RGBA_SRGB_BLOCK,  {VK_FORMAT_BC1_RGBA_SRGB_BLOCK,  false, {5,6,5,1},  128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::S3TC}},
        {VK_FORMAT_BC2_UNORM_BLOCK,      {VK_FORMAT_BC2_UNORM_BLOCK,      false, {5,6,5,4},  128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::S3TC}},
        {VK_FORMAT_BC2_SRGB_BLOCK,       {VK_FORMAT_BC2_SRGB_BLOCK,       false, {5,6,5,4},  128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::S3TC}},
        {VK_FORMAT_BC3_UNORM_BLOCK,      {VK_FORMAT_BC3_UNORM_BLOCK,      false, {5,6,5,8},  128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::S3TC}},
        {VK_FORMAT_BC3_SRGB_BLOCK,       {VK_FORMAT_BC3_SRGB_BLOCK,       false, {5,6,5,8},  128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::S3TC}},
        {VK_FORMAT_BC4_UNORM_BLOCK,      {VK_FORMAT_BC4_UNORM_BLOCK,      false, {8},        64  / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::S3TC}},
        {VK_FORMAT_BC4_SNORM_BLOCK,      {VK_FORMAT_BC4_SNORM_BLOCK,      false, {8},        64  / CHAR_BIT, pixel_format_info::signed_normalized,           pixel_format_info::S3TC}},
        {VK_FORMAT_BC5_UNORM_BLOCK,      {VK_FORMAT_BC5_UNORM_BLOCK,      false, {8,8},      128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::S3TC}},
        {VK_FORMAT_BC5_SNORM_BLOCK,      {VK_FORMAT_BC5_SNORM_BLOCK,      false, {8,8},      128 / CHAR_BIT, pixel_format_info::signed_normalized,           pixel_format_info::S3TC}},
        {VK_FORMAT_BC6H_UFLOAT_BLOCK,    {VK_FORMAT_BC6H_UFLOAT_BLOCK,    false, {16,16,16}, 128 / CHAR_BIT, pixel_format_info::unsigned_float,              pixel_format_info::S3TC}},
        {VK_FORMAT_BC6H_SFLOAT_BLOCK,    {VK_FORMAT_BC6H_SFLOAT_BLOCK,    false, {16,16,16}, 128 / CHAR_BIT, pixel_format_info::signed_float,                pixel_format_info::S3TC}},
        {VK_FORMAT_BC7_UNORM_BLOCK,      {VK_FORMAT_BC7_UNORM_BLOCK,      false, {4,4,4,0},  128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::S3TC}},
        {VK_FORMAT_BC7_SRGB_BLOCK,       {VK_FORMAT_BC7_SRGB_BLOCK,       false, {4,4,4,0},  128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::S3TC}},
        
        {VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,   {VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,   false, {8,8,8},   64  / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ETC}},
        {VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,    {VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,    false, {8,8,8},   64  / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ETC}},
        {VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, {VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, false, {8,8,8,1}, 64  / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ETC}},
        {VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,  {VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,  false, {8,8,8,1}, 64  / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ETC}},
        {VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, {VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, false, {8,8,8,8}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ETC}},
        {VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,  {VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,  false, {8,8,8,8}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ETC}},
        {VK_FORMAT_EAC_R11_UNORM_BLOCK,       {VK_FORMAT_EAC_R11_UNORM_BLOCK,       false, {11},      64  / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ETC}},
        {VK_FORMAT_EAC_R11_SNORM_BLOCK,       {VK_FORMAT_EAC_R11_SNORM_BLOCK,       false, {11},      64  / CHAR_BIT, pixel_format_info::signed_normalized,           pixel_format_info::ETC}},
        {VK_FORMAT_EAC_R11G11_UNORM_BLOCK,    {VK_FORMAT_EAC_R11G11_UNORM_BLOCK,    false, {11,11},   128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ETC}},
        {VK_FORMAT_EAC_R11G11_SNORM_BLOCK,    {VK_FORMAT_EAC_R11G11_SNORM_BLOCK,    false, {11,11},   128 / CHAR_BIT, pixel_format_info::signed_normalized,           pixel_format_info::ETC}},
        
        {VK_FORMAT_ASTC_4x4_UNORM_BLOCK,   {VK_FORMAT_ASTC_4x4_UNORM_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_4x4_SRGB_BLOCK,    {VK_FORMAT_ASTC_4x4_SRGB_BLOCK,    false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_5x4_UNORM_BLOCK,   {VK_FORMAT_ASTC_5x4_UNORM_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_5x4_SRGB_BLOCK,    {VK_FORMAT_ASTC_5x4_SRGB_BLOCK,    false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_5x5_UNORM_BLOCK,   {VK_FORMAT_ASTC_5x5_UNORM_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_5x5_SRGB_BLOCK,    {VK_FORMAT_ASTC_5x5_SRGB_BLOCK,    false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_6x5_UNORM_BLOCK,   {VK_FORMAT_ASTC_6x5_UNORM_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_6x5_SRGB_BLOCK,    {VK_FORMAT_ASTC_6x5_SRGB_BLOCK,    false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_6x6_UNORM_BLOCK,   {VK_FORMAT_ASTC_6x6_UNORM_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_6x6_SRGB_BLOCK,    {VK_FORMAT_ASTC_6x6_SRGB_BLOCK,    false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_8x5_UNORM_BLOCK,   {VK_FORMAT_ASTC_8x5_UNORM_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_8x5_SRGB_BLOCK,    {VK_FORMAT_ASTC_8x5_SRGB_BLOCK,    false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_8x6_UNORM_BLOCK,   {VK_FORMAT_ASTC_8x6_UNORM_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_8x6_SRGB_BLOCK,    {VK_FORMAT_ASTC_8x6_SRGB_BLOCK,    false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_8x8_UNORM_BLOCK,   {VK_FORMAT_ASTC_8x8_UNORM_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_8x8_SRGB_BLOCK,    {VK_FORMAT_ASTC_8x8_SRGB_BLOCK,    false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x5_UNORM_BLOCK,  {VK_FORMAT_ASTC_10x5_UNORM_BLOCK,  false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x5_SRGB_BLOCK,   {VK_FORMAT_ASTC_10x5_SRGB_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x6_UNORM_BLOCK,  {VK_FORMAT_ASTC_10x6_UNORM_BLOCK,  false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x6_SRGB_BLOCK,   {VK_FORMAT_ASTC_10x6_SRGB_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x8_UNORM_BLOCK,  {VK_FORMAT_ASTC_10x8_UNORM_BLOCK,  false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x8_SRGB_BLOCK,   {VK_FORMAT_ASTC_10x8_SRGB_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x10_UNORM_BLOCK, {VK_FORMAT_ASTC_10x10_UNORM_BLOCK, false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x10_SRGB_BLOCK,  {VK_FORMAT_ASTC_10x10_SRGB_BLOCK,  false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_12x10_UNORM_BLOCK, {VK_FORMAT_ASTC_12x10_UNORM_BLOCK, false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_12x10_SRGB_BLOCK,  {VK_FORMAT_ASTC_12x10_SRGB_BLOCK,  false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_12x12_UNORM_BLOCK, {VK_FORMAT_ASTC_12x12_UNORM_BLOCK, false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_12x12_SRGB_BLOCK,  {VK_FORMAT_ASTC_12x12_SRGB_BLOCK,  false, {}, 128 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::ASTC}},
        

        {VK_FORMAT_G8B8G8R8_422_UNORM, {VK_FORMAT_G8B8G8R8_422_UNORM, false, {8,16,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_B8G8R8G8_422_UNORM, {VK_FORMAT_B8G8R8G8_422_UNORM, false, {8,16,8}, 32 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        
        {VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, {VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        {VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,  {VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,  false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM, {VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM, false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        {VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,  {VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,  false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM, {VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM, false, {8,8,8}, 24 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        
        {VK_FORMAT_R10X6_UNORM_PACK16,                 {VK_FORMAT_R10X6_UNORM_PACK16,                 true, {10},          16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_R10X6G10X6_UNORM_2PACK16,           {VK_FORMAT_R10X6G10X6_UNORM_2PACK16,           true, {10,10},       32 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16, {VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16, true, {10,10,10,10}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        
        {VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16, {VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16, true, {10,20,10}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16, {VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16, true, {10,20,10}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        
        {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16, true, {10,10,10}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,  {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,  true, {10,10,10}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16, true, {10,10,10}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,  {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,  true, {10,10,10}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, {VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16, true, {10,10,10}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        
        {VK_FORMAT_R12X4_UNORM_PACK16,                 {VK_FORMAT_R12X4_UNORM_PACK16,                 true, {12},          16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_R12X4G12X4_UNORM_2PACK16,           {VK_FORMAT_R12X4G12X4_UNORM_2PACK16,           true, {12,12},       32 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16, {VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16, true, {12,12,12,12}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized}},

        {VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16, {VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16, true, {12,24,12}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16, {VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16, true, {12,24,12}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        
        {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16, true, {12,12,12}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,  {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,  true, {12,12,12}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16, true, {12,12,12}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,  {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,  true, {12,12,12}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, {VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16, true, {12,12,12}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        
        {VK_FORMAT_G16B16G16R16_422_UNORM, {VK_FORMAT_G16B16G16R16_422_UNORM, true, {16, 32, 16}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_B16G16R16G16_422_UNORM, {VK_FORMAT_B16G16R16G16_422_UNORM, true, {16, 32, 16}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        
        {VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM, {VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM, true, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        {VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,  {VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,  true, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM, {VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM, true, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        {VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,  {VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,  true, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM, {VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM, true, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane3}},
        
        {VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,                  {VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,                  false, {8,8,8},    24 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16, {VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16, true,  {10,10,10}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16, {VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16, true,  {12,12,12}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        {VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,               {VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,               false, {16,16,16}, 48 / CHAR_BIT, pixel_format_info::unsigned_normalized, pixel_format_info::none, pixel_format_info::plane2}},
        

        {VK_FORMAT_A4R4G4B4_UNORM_PACK16, {VK_FORMAT_A4R4G4B4_UNORM_PACK16, true, {4,4,4,4}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        {VK_FORMAT_A4B4G4R4_UNORM_PACK16, {VK_FORMAT_A4B4G4R4_UNORM_PACK16, true, {4,4,4,4}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        

        {VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,   {VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,   {VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,   {VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,   {VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,   {VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,   {VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,   {VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,   {VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,   false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,  {VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,  false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,  {VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,  false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,  {VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,  false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK, {VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK, false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK, {VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK, false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        {VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK, {VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK, false, {}, 128 / CHAR_BIT, pixel_format_info::signed_float, pixel_format_info::ASTC}},
        
        {VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, {VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, false, {4,4,3,3}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::PVRTC}},
        {VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG, {VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG, false, {4,4,3,3}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::PVRTC}},
        {VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG, {VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG, false, {4,4,3,3}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::PVRTC}},
        {VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG, {VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG, false, {4,4,3,3}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized,         pixel_format_info::PVRTC}},
        {VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,  {VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,  false, {4,4,3,3}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::PVRTC}},
        {VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,  {VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,  false, {4,4,3,3}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::PVRTC}},
        {VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,  {VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,  false, {4,4,3,3}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::PVRTC}},
        {VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,  {VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,  false, {4,4,3,3}, 64 / CHAR_BIT, pixel_format_info::unsigned_normalized_encoded, pixel_format_info::PVRTC}},
        

        {VK_FORMAT_R16G16_SFIXED5_NV, {VK_FORMAT_R16G16_SFIXED5_NV, false, {16,16}, 32 / CHAR_BIT, pixel_format_info::signed_integer}},
        

        {VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR, {VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR, true, {5,5,5,1}, 16 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
        

        {VK_FORMAT_A8_UNORM_KHR, {VK_FORMAT_A8_UNORM_KHR, false, {0,0,0,8}, 8 / CHAR_BIT, pixel_format_info::unsigned_normalized}},
    };
}
