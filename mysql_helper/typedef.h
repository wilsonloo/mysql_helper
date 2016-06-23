#ifndef EVL_COMMON_DEF_TYPEDEF_H_
#define EVL_COMMON_DEF_TYPEDEF_H_

#include <iostream>
#include <boost/cstdint.hpp>

#ifndef NULL
#define NULL ((void*)(0))
#endif

namespace evl
{
	//
	// ���ڼ�������Ԫ�صĸ���.
	// NOTE:
	//		ͨ�����µ�д�������ã�
	//		char buf[MAX_COUNT];
	//		for (int n = 0; n < MAX_COUNT; ++n) { buf[n] = 0; }
	//		��Ӧ����forѭ������ArraySize(buf)�滻MAX_COUNT���Ż���ơ�
	template<typename T, int N>
	inline size_t ArraySize(T(&array)[N])
	{
		return sizeof(array) / sizeof(array[0]);
	}

	namespace def
	{
#pragma pack(push,1)
		typedef boost::int8_t		int8;
		typedef boost::int16_t		int16;
		typedef boost::int32_t		int32;
		typedef boost::int64_t		int64;

		typedef boost::uint8_t		uint8;
		typedef boost::uint16_t		uint16;
		typedef boost::uint32_t		uint32;
		typedef boost::uint64_t		uint64;

		typedef char				Byte;
		typedef float				Float;


#pragma pack(pop)

#ifdef EVL_LIBRARY_SOURCE

#define EVL_DLL_EXPORT extern "C" __declspec(dllexport)
#define EVL_DLL_DESC __declspec(dllexport)

#else

#define EVL_DLL_EXPORT extern "C" __declspec(dllimport)
#define EVL_DLL_DESC __declspec(dllimport)

#endif
	} // namespace def
} // namespace evl

#endif // EVL_COMMON_DEF_TYPEDEF_H_