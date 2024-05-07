#pragma once
#include <vector>
#include <memory>
#include <utility>

namespace pIOn::utils
{
	template<typename T, size_t CHUNCK_SIZE = 100>
	class ObjectPool
	{
	public:
		ObjectPool() = default;
		ObjectPool(const ObjectPool&) = delete;
		ObjectPool& operator=(const ObjectPool&) = delete;
		ObjectPool(ObjectPool&&) = delete;
		ObjectPool& operator=(ObjectPool&&) = delete;

		template<typename... Args>
		[[nodiscard]] T* allocate(Args&&... args)
		{
			T* obj = getRaw();
			new (obj) T(std::forward<Args>(args)...);
			return obj;
		}

		void deallocate(T* obj) noexcept
		{
			std::destroy_at(obj);
			free_ptrs.push_back(obj);
		}

		void freeSpace() noexcept
		{
			buffers.clear();
			free_ptrs.clear();
		}

		~ObjectPool() noexcept
		{
			freeSpace();
		}
	private:
		T* getRaw()
		{
			if (free_ptrs.empty()) {
				expandMem();
			}

			T* obj = free_ptrs.back();
			free_ptrs.pop_back();
			return obj;
		}

		class Buffer final
		{
		public:
			Buffer() = delete;
			Buffer(const Buffer&) = delete;
			Buffer& operator=(const Buffer&) = delete;

			Buffer(size_t num = CHUNCK_SIZE)
				: buffer_(reinterpret_cast<T*> (::operator new (sizeof(T) * num)))
				, size_(num)
			{
			}

			Buffer(Buffer&& other) noexcept
			{
				this->operator=(std::move(other));
			}

			Buffer& operator=(Buffer&& other) noexcept
			{
				if (this != std::addressof(other)) {
					size_ = std::exchange(other.size_, 0);
					buffer_ = std::exchange(other.buffer_, nullptr);
				}

				return *this;
			}

			~Buffer() noexcept 
			{
				if (buffer_) {
					::operator delete(buffer_);
				}
			}

			size_t size() const noexcept
			{
				return size_;
			}

			T* getBuffer() noexcept
			{
				return buffer_;
			}

		private:
			T* buffer_{ nullptr };
			size_t size_{ 0 };
		};

		std::vector<Buffer> buffers;
		std::vector<T*> free_ptrs;

		void expandMem()
		{
			buffers.emplace_back(CHUNCK_SIZE);
			Buffer& buf = buffers.back();

			for (size_t i = 0; i < buf.size(); ++i) {
				free_ptrs.emplace_back(&buf.getBuffer()[i]);
			}
		}
	};
}