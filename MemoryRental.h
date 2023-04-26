// Copyright (c) 2022, Gold Smith
// Released under the MIT license
// https ://opensource.org/licenses/mit-license.php
#pragma once
//���̃N���X��Windows��p�ł��B
// ********�g�p������ݒ�***********
#define USING_CRITICAL_SECTION// �N���e�B�J���Z�N�V�����g�p����ꍇ
#define CONFIRM_POINT    // �m�F�K�v�̏ꍇ
#define USING_DEBUG_STRING		// �f�X�g���N�^�Ő������m�F
// ******�����ݒ�I���*************

#ifdef USING_DEBUG_STRING
#define NOMINMAX
#include <Windows.h>
#include < algorithm >
#include <sstream>
#endif // USING_DEBUG_STRING

#ifdef USING_CRITICAL_SECTION
#include <synchapi.h>
#endif // USING_CRITICAL_SECTION

#include <Windows.h>
#include <exception>
#include <mutex>
#include <atomic>
#include <string>
#include <iostream>

template <class T>class MemoryRental
{
public:
	MemoryRental() = delete;
	//sizeIn��2�ׂ̂���Ŗ����Ă͂Ȃ�܂���B
	MemoryRental(T* const pBufIn, size_t sizeIn)
		:ppBuf(nullptr)
		, size(sizeIn)
		, front(0)
		, end(0)
		, mask(sizeIn - 1)
#ifdef USING_CRITICAL_SECTION
		, cs{}
#endif // USING_CRITICAL_SECTION
	{
		if ((sizeIn & (sizeIn - 1)) != 0)
		{
			std::string estr("Err! The MemoryRental must be Power of Two.\r\n");
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(estr.c_str());
#endif// USING_DEBUG_STRING
			std::cerr << estr;
			throw std::invalid_argument(estr);
		}

#ifdef USING_CRITICAL_SECTION
		(void)InitializeCriticalSectionAndSpinCount(&cs, 3000);
#endif // USING_CRITICAL_SECTION

		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}
	MemoryRental(MemoryRental& obj) = delete;
	MemoryRental(MemoryRental&& obj) = delete;
	~MemoryRental()
	{
#ifdef USING_CRITICAL_SECTION
		DeleteCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION

#ifdef USING_DEBUG_STRING
		std::stringstream ss;
		ss << "MemoryRental "
			<< "\"" << strID.c_str() << "\" "
			<< "log. front;" << std::to_string(front)
			<< " end:" << std::to_string(end)
			<< " end-front:" << std::to_string(end - front)
			<< " size:" << std::to_string(size)
			<< " Maximum peak usage:" << std::to_string(max_using)
			<< "\r\n";
		OutputDebugStringA(ss.str().c_str());
#endif // USING_DEBUG_STRING
		delete[]ppBuf;
	}

	//sizeIn��2�ׂ̂���Ŗ����Ă͂Ȃ�܂���B
	void ReInitialize(T* pBufIn, size_t sizeIn)
	{
		delete[]ppBuf;
		ppBuf = nullptr;
		size = sizeIn;
		front = 0;
		end = 0;
		mask = sizeIn - 1;

		if ((sizeIn & (sizeIn - 1)) != 0)
		{
			std::string estr("Err! The MemoryRental must be Power of Two.\r\n");
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(estr.c_str());
#endif// USING_DEBUG_STRING
			std::cerr << estr;
			throw std::invalid_argument(estr);
		}

		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}

	T* Lend()
	{
#ifdef USING_CRITICAL_SECTION
		EnterCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION
#ifdef CONFIRM_POINT
		if (front + size < end)
		{
			std::string estr("Err! \"" + strID
				+ "\" MemoryRental.Lend (front("
				+ std::to_string(front)
				+ ")& mask) + 1 == (end("
				+ std::to_string(end)
				+ " & mask)\r\n");
			std::cerr << estr;
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(estr.c_str());
#endif// USING_DEBUG_STRING
			throw std::runtime_error(estr); // ��O���o
		}
#endif // !CONFIRM_POINT
		T** ppT = &ppBuf[end & mask];
		++end;
#ifdef USING_CRITICAL_SECTION
		LeaveCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION
#ifdef USING_DEBUG_STRING
		max_using.store(
			(std::max)(end.load() - front.load(), max_using.load()));
#endif // USING_DEBUG_STRING
		return *ppT;
	}

	void Return(T* pT)
	{
#ifdef USING_CRITICAL_SECTION
		EnterCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION
#ifdef CONFIRM_POINT
		if (front == end + size)
		{
			std::string estr("Err! \"" + strID
				+ "\" MemoryRental.Return(front("
				+ std::to_string(front)
				+ ") & mask) + 1 == ( end("
				+ std::to_string(end)
				+ ") & mask)\r\n");
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(estr.c_str());
#endif// USING_DEBUG_STRING
			std::cerr << estr;
			throw std::runtime_error(estr); // ��O���o
		}

#endif // !CONFIRM_POINT
		ppBuf[front & mask] = pT;
		++front;
#ifdef USING_CRITICAL_SECTION
		LeaveCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION
	}

	void DebugString(const std::string str)
	{
#ifdef USING_DEBUG_STRING
		strID = str;
#endif // USING_DEBUG_STRING
	}

protected:
	T** ppBuf;
	std::atomic_size_t size;
	std::atomic_size_t front;
	std::atomic_size_t end;
	std::atomic_size_t mask;
#ifdef USING_CRITICAL_SECTION
	CRITICAL_SECTION cs;
#endif // USING_CRITICAL_SECTION

#ifdef USING_DEBUG_STRING
	std::atomic_size_t max_using;
	std::string strID;
#endif // USING_DEBUG_STRING
};


#ifdef USING_CRITICAL_SECTION
#undef USING_CRITICAL_SECTION
#endif // USING_CRITICAL_SECTION

#ifdef CONFIRM_POINT
#undef CONFIRM_POINT
#endif // CONFIRM_POINT

#ifdef USING_DEBUG_STRING
#undef NOMINMAX
#undef USING_DEBUG_STRING
#endif // USING_DEBUG_STRING

