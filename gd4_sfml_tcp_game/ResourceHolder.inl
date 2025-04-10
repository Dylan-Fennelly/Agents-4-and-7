/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "ResourceHolder.hpp"

template <typename Identifier, typename Resource>
void ResourceHolder<Identifier, Resource>::Load(Identifier id, const std::string& filename)
{
	// Create and load resource
	std::unique_ptr<Resource> resource(new Resource());
	if (!resource->loadFromFile(filename))
		throw std::runtime_error("ResourceHolder::load - Failed to load " + filename);

	auto inserted = m_resource_map.insert(std::make_pair(id, std::move(resource)));
	assert(inserted.second);
}

template <typename Identifier, typename Resource>
Resource& ResourceHolder<Identifier, Resource>::Get(Identifier id)
{
	auto found = m_resource_map.find(id);
	assert(found != m_resource_map.end());
	return *found->second;
}

template <typename Identifier, typename Resource>
const Resource& ResourceHolder<Identifier, Resource>::Get(Identifier id) const
{
	auto found = m_resource_map.find(id);
	assert(found != m_resource_map.end());
	return *found->second;
}

template <typename Identifier, typename Resource>
template <typename Parameter>
void ResourceHolder<Identifier, Resource>::Load(Identifier id, const std::string& filename, const Parameter& second_param)
{
	std::unique_ptr<Resource> resource(new Resource());
	if (!resource->loadFromFile(filename, second_param))
		throw std::runtime_error("ResourceHolder::load - Failed to load " + filename);

	auto inserted = m_resource_map.insert(std::make_pair(id, std::move(resource)));
	assert(inserted.second);
}
