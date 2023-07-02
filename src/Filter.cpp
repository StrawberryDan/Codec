//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Codec/Filter.hpp"
/// Strawberry Libraries
#include "Strawberry/Core/Assert.hpp"
/// Standard Library
#include <memory>


namespace Strawberry::Codec
{
	Filter::Filter()
			: mFilterContext(nullptr) {}


	Filter::Filter(Filter&& rhs)
			: mFilterContext(std::exchange(rhs.mFilterContext, nullptr)) {}


	Filter& Filter::operator=(Filter&& rhs)
	{
		if (this != &rhs)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(rhs));
		}

		return *this;
	}


	Filter::~Filter()
	{

	}


	AVFilterContext*& Filter::operator*()
	{
		return mFilterContext;
	}


	const AVFilterContext* Filter::operator*() const
	{
		return mFilterContext;
	}


	void Filter::Link(Filter& consumer, unsigned int srcPad, unsigned int dstPad)
	{
		auto result = avfilter_link(**this, srcPad, *consumer, dstPad);
		Core::Assert(result == 0);
	}
}