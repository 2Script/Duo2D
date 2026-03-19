#include "sirius/vulkan/display/surface.hpp"

namespace acma::vk {
    result<surface> surface::create(GLFWwindow* w) noexcept {
        surface ret{};
        __D2D_VULKAN_VERIFY(glfwCreateWindowSurface(impl::vulkan_instance(), w, nullptr, &ret.handle));
        return ret;
    }
}

namespace acma::vk {
	surface::~surface() noexcept {
		if(this->handle) 
			vkDestroySurfaceKHR(impl::vulkan_instance(), this->handle, nullptr);
	}


	surface::surface(surface&& other) noexcept{
        if(this->handle && this->handle != other.handle) 
            vkDestroySurfaceKHR(impl::vulkan_instance(), this->handle, nullptr);
		this->handle = std::exchange(other.handle, VK_NULL_HANDLE);
	}

    surface& surface::operator=(surface&& other) noexcept{
        if(this->handle && this->handle != other.handle) 
            vkDestroySurfaceKHR(impl::vulkan_instance(), this->handle, nullptr);
		this->handle = std::exchange(other.handle, VK_NULL_HANDLE);
        return *this;
	}
}