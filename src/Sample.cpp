#include "Codec/Sample.hpp"



#include "Standard/Assert.hpp"
#include <cstring>
#include <utility>



namespace Strawberry::Codec
{
	Samples::Samples()
	    : mLeft()
	    , mRight()
	{

	}



	Samples::Samples(Channel left, Channel right)
	    : mLeft(std::move(left))
	    , mRight(std::move(right))
	{
	    Assert(mLeft.size() == mRight.size());
	}



	Samples::Samples(const void* left, const void* right, std::size_t size)
	    : mLeft()
	    , mRight()
	{
	    Assert(size % sizeof(Sample) == 0);
	    mLeft.resize(size / sizeof(Sample));
	    mRight.resize(size / sizeof(Sample));
	    memcpy(mLeft.data(), left, size);
	    memcpy(mRight.data(), right, size);
	}



	void Samples::Append(const Samples& other)
	{
	    mLeft.insert( mLeft.end(),  other.mLeft.begin(),  other.mLeft.end());
	    mRight.insert(mRight.end(), other.mRight.begin(), other.mRight.end());
	}



	void Samples::Append(Samples&& other)
	{
	    mLeft.insert( mLeft.end(),  other.mLeft.begin(),  other.mLeft.end());
	    mRight.insert(mRight.end(), other.mRight.begin(), other.mRight.end());
	}



	Samples::SplitSamples Samples::Split(std::size_t count) const
	{
	    Samples stem, leaf;
	    std::size_t stemSize = count * (Size() / count);

	    stem.mLeft  = {mLeft.begin(),  mLeft.begin()  + stemSize};
	    stem.mRight = {mRight.begin(), mRight.begin() + stemSize};

	    leaf.mLeft  = {mLeft.begin()  + stemSize,  mLeft.end()};
	    leaf.mRight = {mRight.begin() + stemSize, mRight.end()};

	    std::size_t splitCount = stemSize / count;
	    std::vector<Samples> splits;

	    for (int i = 0; i < splitCount; ++i)
	    {
	        auto leftPtr  = mLeft.data()  + (count * i);
	        auto rightPtr = mRight.data() + (count * i);
	        splits.emplace_back(leftPtr, rightPtr, count);
	    }

	    std::size_t sizeCheck = leaf.Size();
	    for (const auto& split : splits)
	    {
	        sizeCheck += split.Size();
	    }
	    Assert(sizeCheck == Size());

	    return {splits, leaf};
	}



	void Samples::Multiply(float factor)
	{
	    for (auto& s : mLeft)
	    {
	        s *= factor;
	    }

	    for (auto& s : mRight)
	    {
	        s *= factor;
	    }
	}
}