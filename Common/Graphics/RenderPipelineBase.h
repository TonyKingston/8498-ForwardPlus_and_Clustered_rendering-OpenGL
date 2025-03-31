#pragma once
#include <unordered_map>
#include <any>
#include <typeindex>
#include <stdexcept>
#include <vector>
#include <memory>
#include <algorithm>
#include <ranges>
#include <Common.h>
#include "RenderPassBase.h"

namespace NCL {
    namespace Rendering {
        class RenderPassDepedency;

        using ResourceId = std::size_t;

        struct ResourceDescriptor {
            ResourceId         id = 0;
            std::string        name;
            std::type_index    type = std::type_index(typeid(void));
        };

        class PipelineResourceRegistry
        {
        public:
            // Create/store a resource of type T (e.g. GLuint, struct, etc.) with a given name
            // Returns the descriptor that you can pass around
            template<typename T>
            ResourceDescriptor Store(const std::string& name, const T& resource) {
                ResourceId newId = GenerateResourceId();

                ResourceDescriptor desc{ newId, name, std::type_index(typeid(T)) };
                // Put the resource in std::any
                m_data[newId] = resource;
                // Also store the descriptor
                m_descriptors[newId] = desc;

                return desc;
            }

            // Overload if you only want to store by descriptor
            template<typename T>
            ResourceDescriptor Store(ResourceDescriptor desc, const T& resource) {
                // Make sure the descriptor's type matches T
                desc.type = std::type_index(typeid(T));
                m_data[desc.id] = resource;
                m_descriptors[desc.id] = desc;
                return desc;
            }

            // Retrieve a resource by descriptor
            template<typename T>
            T& Get(const ResourceDescriptor& desc) {
                // Optional: Check type safety (desc.type == typeid(T)?)
                if (desc.type != std::type_index(typeid(T))) {
                    LOG_ERROR("Type mismatch: resource {} was stored with type {} but requested as {}",
                        desc.name, std::string(desc.type.name()), typeid(T).name());
                    NCL_ASSERT(desc.type == std::type_index(typeid(T)));
                }

                auto it = m_data.find(desc.id);
                if (it == m_data.end()) {
                    LOG_ERROR("Resource not found: {}", desc.name);
                }

                // Perform std::any_cast
                return std::any_cast<T&>(it->second);
            }

            // Convenience: retrieve by name + type
            template<typename T>
            T& Get(const std::string& name) {
                ResourceId id = FindIdByName(name);
                auto it = m_descriptors.find(id);
                if (it == m_descriptors.end()) {
                    NCL_ASSERT(true);
                }
                return get<T>(it->second);
            }

            //template<typename T>
            //bool Has() const {
            //    return m_data.contains(std::type_index(typeid(T)));
            //}

            // If you want to remove a resource
            void Remove(const ResourceDescriptor& desc) {
                m_data.erase(desc.id);
                m_descriptors.erase(desc.id);
            }

            // Debug or enumerating
            void ListAllResources() const {
                for (auto& kv : m_descriptors) {
                    auto& d = kv.second;
                    //LOG_INFO("{}", d);
                    /*std::cout << "ResourceId=" << d.id
                        << " Name='" << d.name
                        << "' Type=" << d.type.name() << std::endl;*/
                }
            }

        private:
            ResourceId GenerateResourceId() {
                return m_nextId++;
            }

            ResourceId FindIdByName(const std::string& name) const {
                for (auto& kv : m_descriptors) {
                    if (kv.second.name == name) {
                        return kv.first;
                    }
                }
                throw std::runtime_error("Resource name not found: " + name);
            }

            // Data storage: resource ID -> std::any
            std::unordered_map<ResourceId, std::any> m_data;

            // Metadata: resource ID -> descriptor
            std::unordered_map<ResourceId, ResourceDescriptor> m_descriptors;

            ResourceId m_nextId = 1;
        };


        /*class PipelineResourceRegistry {
        public:
            template<typename T>
            void Put(T&& resource) {
                resources[std::type_index(typeid(T))] = std::forward<T>(resource);
            }

            template<typename T>
            T* Get() {
                auto it = resources.find(std::type_index(typeid(T)));
                if (it != resources.end()) {
                    return std::any_cast<T>(&it->second);
                }
                return nullptr;
            }

            template<typename T>
            bool Has() const {
                return resources.contains(std::type_index(typeid(T)));
            }

        private:
            std::unordered_map<std::type_index, std::any> resources;
        };*/

        class RenderPipeline {
        public:
            RenderPipeline() = default;
            explicit RenderPipeline(uint32_t numPasses) {
                passes.reserve(numPasses);
            }

            RenderPipeline& AddRenderPass(std::shared_ptr<RenderPassBase> pass) {
                passes.push_back(pass);
                bIsDirty = true;
                return *this;
            }
                    
            // Calls Init on all render passes. Even if they aren't enabled
            void InitRenderPasses() {
                for (auto& pass : passes) {
                    pass->Init();
                }
            }

            void Execute() {

                if (bIsDirty) {
                    RebuildDAG();
                    bIsDirty = false;
                }

                auto enabledPasses = passes | std::views::filter([](auto pass) {
                    return pass->IsEnabled();
                 });

                for (auto& pass : enabledPasses) {
                    RenderPassDependency deps = pass->DeclareDependencies();

                    // Ensure required resources exist
                    for (const auto& required : deps.requiredResources) {
                        /*if (!resources.Has<std::any>()) {
                            NCL_FATAL("Missing Required resource!");
                        }*/
                    }

                    // Execute render pass
                    pass->Execute(resources);
                }
            }

        private:

            void RebuildDAG();

            std::vector<std::shared_ptr<RenderPassBase>> passes;
            std::vector<std::shared_ptr<RenderPassBase>> sortedPasses;

            PipelineResourceRegistry resources;

            // If true, we need to rebuild the dependancy graph
            bool bIsDirty{ true };
        };
    }
}
