#pragma once



#include <vector>
#include "Standard/Assert.hpp"



using Sample = float;
using Channel = std::vector<Sample>;



struct SplitSamples;



namespace Strawberry::Codec
{
	class Samples
	{
	public:
	    using SplitSamples = std::pair<std::vector<Samples>, Samples>;

	public:
	    Samples();
	    Samples(Channel  left, Channel  right);
	    Samples(const void* left, const void* right, std::size_t size);

	    template<typename T>
	    inline constexpr Samples(const T* left, const T* right, std::size_t size)
	        : Samples(reinterpret_cast<const void*>(left), reinterpret_cast<const void*>(right), size * sizeof(T))
	    {
	    }

	    [[nodiscard]] inline const Channel& Left()  const { return  mLeft; }
	    [[nodiscard]] inline const Channel& Right() const { return mRight; }

	    void Append(const Samples& other);
	    void Append(Samples&& other);

	    [[nodiscard]] inline std::size_t Size() const { Standard::Assert(mLeft.size() == mRight.size()); return mLeft.size(); }

	    [[nodiscard]] SplitSamples Split(std::size_t count) const;


	    void Multiply(float factor);

	private:
	    Channel mLeft;
	    Channel mRight;
	};
}