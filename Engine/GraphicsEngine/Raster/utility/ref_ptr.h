#pragma once

#include <memory>

////////////////////////////////////////////////////////////////////////////////
/// Special unique_ptr for refcounting internal graphics engine resources.
///
////////////////////////////////////////////////////////////////////////////////

template <class T>
struct Deleter 
{
	void operator()(T* p) {
		p->Release();
	}
};

template <class T>
class ref_ptr : public std::unique_ptr<T, Deleter<T>>
{
public:
	// ctor
	ref_ptr() : unique_ptr() { }
	ref_ptr(nullptr_t) : unique_ptr(nullptr) { }
	explicit ref_ptr(T* p) : unique_ptr(p) { p->Acquire(); }
	ref_ptr(ref_ptr&& rhs) : unique_ptr(rhs) { }

	// copy
	ref_ptr& operator=(nullptr_t) {
		unique_ptr<T, Deleter<T>>::operator=(nullptr);
	}
	ref_ptr& operator=(ref_ptr&& rhs) { }

	// dereference
	T* operator->() {
		return unique_ptr<T, Deleter<T>>::operator->();
	}
	T& operator*() {
		return unique_ptr<T, Deleter<T>>::operator*();
	}

	// reset
	ref_ptr& Reset(nullptr_t) {
		unique_ptr<T, Deleter<T>>::Reset(nullptr);
		return *this;
	}
	ref_ptr& Reset(T* p) {
		unique_ptr<T, Deleter<T>>::reset(p);
		p->Acquire();
		return *this;
	}
};