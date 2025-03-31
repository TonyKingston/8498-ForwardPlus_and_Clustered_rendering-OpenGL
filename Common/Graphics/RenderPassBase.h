#pragma once
#include <vector>
#include <typeindex>
#include <string>

namespace NCL {
	namespace Rendering {
		class RenderPipeline;
		class PipelineResourceRegistry;
		
		struct ResourceHandle {
			std::string name;
			std::type_index type;

			bool operator==(const ResourceHandle& other) const {
				return name == other.name && type == other.type;
			}
		};

		struct RenderPassDependency {
			std::vector<ResourceHandle> requiredResources;
			std::vector<ResourceHandle> producedResources;

			template<typename T>
			void Requires(const std::string& name) { 
				requiredResources.emplace_back(ResourceHandle{ name, typeid(T) }); 
			}

			template<typename T>
			void Produces(const std::string& name) { 
				producedResources.emplace_back(ResourceHandle{ name, typeid(T) }); 
			}
		};
        
		class RenderPassBase {
		public:
			virtual ~RenderPassBase() = default;
			// Called once by RenderPipeline. Prefer initilising any resources here rather than the constructor
			virtual void Init() {};

			// Per-frame processing i.e. put the rendering work here
			virtual void Execute(PipelineResourceRegistry& resources) = 0;

			virtual RenderPassDependency DeclareDependencies() const = 0;

			[[nodiscard]] bool IsEnabled() const { return bIsEnabled; }
			void Enable() { bIsEnabled = true; }
			void Disable() { bIsEnabled = false; }

		private:
			bool bIsEnabled{ true };
		};
	}
}
