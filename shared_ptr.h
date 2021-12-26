/**
 * @file  shared_ptr.h
 * @brief shared_ptr is a minimal implementation of smart pointer, a subset of the C++11 std::shared_ptr or boost::shared_ptr.
 *
 * Copyright (c) 2021-2022 YuxiangZhu (18842550358@163.com)
 *
 */
#pragma once
#include <exception>
#include <algorithm>

class _Ref_count_base
{
public:
	constexpr _Ref_count_base() noexcept :
		_Count(nullptr)
	{
	}

	_Ref_count_base(const _Ref_count_base& _Ptr) noexcept :
		_Count(_Ptr._Count)
	{
	}

	long _Use_count() noexcept
	{
		return nullptr == _Count ? 0 : (*_Count);
	}

	template <typename _Uy>
	void _Incwref(_Uy* _Ptr)
	{
		if (nullptr != _Ptr)
		{
			if (nullptr == _Count)
			{
				try
				{
					_Count = new long(1); //may throw std::bad_alloc
				}
				catch (std::bad_alloc&)
				{
					delete _Ptr;
					_Ptr = nullptr;
					throw;
				}
			}
			else
			{
				++(*_Count);
			}
		}
	}

	template <typename _Uy>
	void _Decwref(_Uy* _Ptr) noexcept
	{
		if (nullptr != _Count)
		{
			--(*_Count);
			if (0 == *_Count)
			{
				delete _Ptr;
				delete _Count;
				_Ptr = nullptr;
				_Count = nullptr;
			}
		}
	}

private:
	long* _Count{ nullptr };
};

template <typename _Ty>
class _Ptr_base
{
public:
	using _Elem_type = _Ty;

	_Ptr_base(const _Ptr_base&) = delete;
	_Ptr_base& operator=(const _Ptr_base&) = delete;
	
	long use_count() const noexcept
	{
		return nullptr == _Rep ? 0 : _Rep->_Use_count();
	}

	_Elem_type* get() const noexcept
	{
		return _Ptr;
	}

	explicit operator bool() const noexcept
	{
		return _Ptr != nullptr;
	}

	bool unique() const noexcept
	{
		return (1 == this->use_count());
	}

	_Elem_type* operator->() const noexcept
	{
		return this->_Ptr;
	}

	_Elem_type& operator*() const noexcept
	{
		return *this->_Ptr;
	}
protected:
	constexpr _Ptr_base() noexcept = default;
	virtual ~_Ptr_base() = default;

	void _Set_enable_shared() 
	{
		try
		{
			_Rep = new _Ref_count_base();
		}
		catch (std::bad_alloc&)
		{
			_Rep = nullptr;
			throw;
		}
	}

	void _Copy_point_from(const _Elem_type* _Ptr) noexcept
	{
		this->_Ptr = const_cast<_Elem_type*>(_Ptr);
		this->_Incref();
	}

	template <typename _Ty2>
	void _Move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
	{
		this->_Ptr = static_cast<_Ptr_base<_Ty>::_Elem_type*>(_Right._Ptr);
		this->_Rep = _Right._Rep;
		_Right._Ptr = nullptr;
		_Right._Rep = nullptr;
	}

	template <typename _Ty2>
	void _Copy_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept
	{
		_Other._Incref();
		this->_Ptr = static_cast<_Ptr_base<_Ty>::_Elem_type*>(_Other._Ptr);
		this->_Rep = _Other._Rep;
	}

	template <typename _Ty2>
	void _Alias_construct_from(const _Ptr_base<_Ty2>& _Other) noexcept
	{
		_Other._Incref();
		this->_Ptr = static_cast<_Ptr_base<_Ty>::_Elem_type*>(_Other._Ptr);
		this->_Rep = _Other._Rep;
	}

	template <typename _Ty2>
	void _Alias_construct_from(const _Ptr_base<_Ty2>& _Other, _Elem_type* _Px) noexcept
	{
		_Other._Incref();
		this->_Ptr = _Px;
		this->_Rep = _Other._Rep;
	}

	template <typename _Ty2>
	void _Alias_move_construct_from(_Ptr_base<_Ty2>&& _Right, _Elem_type* _Px) noexcept
	{
		this->_Ptr = _Px;
		this->_Rep = _Right._Rep;
		_Right._Ptr = nullptr;
		_Right._Rep = nullptr;
	}

	template <typename _Ty2>
	void _Alias_move_construct_from(_Ptr_base<_Ty2>&& _Right) noexcept
	{
		this->_Ptr = static_cast<_Ptr_base<_Ty>::_Elem_type*>(_Right._Ptr);
		this->_Rep = _Right._Rep;
		_Right._Ptr = nullptr;
		_Right._Rep = nullptr;
	}

	void _Incref() const
	{
		if (nullptr != _Rep)
		{
			_Rep->_Incwref(this->_Ptr);
		}
	}

	void _Decref() noexcept
	{
		if (nullptr != _Rep)
		{
			_Rep->_Decwref(this->_Ptr);
		}
	}

	void _Swap(_Ptr_base& _Right) noexcept
	{
		std::swap(this->_Ptr, _Right._Ptr);
		std::swap(this->_Rep, _Right._Rep);
	}
protected:
	_Elem_type* _Ptr{ nullptr };
	_Ref_count_base* _Rep{ nullptr };
};

template <typename _Ty>
class shared_ptr : public _Ptr_base<_Ty> //class for reference counted resource management
{
private:
	using _Shared_base = _Ptr_base<_Ty>;
public:
	using typename _Shared_base::_Elem_type;

	constexpr shared_ptr() noexcept = default;

	constexpr shared_ptr(nullptr_t) noexcept {}

	explicit shared_ptr(_Ty* _Ptr) noexcept
	{
		this->_Set_enable_shared();
		this->_Copy_point_from(_Ptr);
	}

	shared_ptr(const shared_ptr& _Other) noexcept
	{
		this->_Copy_construct_from(_Other);
	}

	shared_ptr(shared_ptr&& _Right) noexcept
	{
		this->_Move_construct_from(std::move(_Right));
	}
	
	template <typename _Uy>
	shared_ptr(const shared_ptr<_Uy>& _Other) noexcept
	{
		this->_Copy_construct_from(_Other);
	}

	template <typename _Uy>
	shared_ptr(const shared_ptr<_Uy>&& _Right) noexcept
	{
		this->_Alias_move_construct_from(std::move(_Right));
	}

	template <typename _Uy>
	shared_ptr(const shared_ptr<_Uy>& _Other, _Elem_type* _Ptr) noexcept
	{
		this->_Alias_construct_from(_Other, _Ptr);
	}

	template <typename _Uy>
	shared_ptr(const shared_ptr<_Uy>&& _Right, _Elem_type* _Ptr) noexcept
	{
		this->_Alias_move_construct_from(std::move(_Right), _Ptr);
	}

	~shared_ptr() noexcept
	{
		this->_Decref();
	}

	shared_ptr& operator=(const shared_ptr& _Other) noexcept
	{
		shared_ptr(_Other).swap(*this);
		return *this;
	}

	shared_ptr& operator=(shared_ptr&& _Right) noexcept
	{
		shared_ptr(std::move(_Right)).swap(*this);
		return *this;
	}

	template <typename _Ty2>
	shared_ptr& operator=(const shared_ptr<_Ty2>& _Other) noexcept
	{
		shared_ptr(_Other).swap(*this);
		return *this;
	}

	template <typename _Ty2>
	shared_ptr& operator=(shared_ptr<_Ty2>&& _Right) noexcept
	{
		shared_ptr(std::move(_Right)).swap(*this);
		return *this;
	}

	void swap(shared_ptr& _Other) noexcept
	{
		this->_Swap(_Other);
	}
	
	void reset() noexcept //release resource and convert to empty shared_ptr
	{
		shared_ptr().swap(*this);
	}

	template <typename _Uy>
	void reset(_Uy* _Ptr) noexcept
	{
		shared_ptr(_Ptr).swap(*this);
	}
private:
	template <typename _Ty, typename ..._Args>
	friend shared_ptr<_Ty> make_shared(_Args&& ..._Right) noexcept;
};

template <typename _Ty, typename ..._Args>
shared_ptr<_Ty> make_shared(_Args&& ..._Right) noexcept
{
	return shared_ptr<_Ty>(new _Ty(std::forward<_Args>(_Right)...));
}