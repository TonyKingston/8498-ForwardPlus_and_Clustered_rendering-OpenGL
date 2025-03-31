#pragma once
#include "ShaderBase.h"

namespace NCL {
    namespace Rendering {

        enum class FillMode { Solid, Wireframe, Point };
        enum class DepthTest { Off, On };
        enum class BlendMode { None, AlphaBlend, Test};
        enum class CullMode { None, Backface, Frontface };

        class PipelineState {
        public:

			struct PipelineInfo {
				ShaderBase* shader{ nullptr };
				//Topology topology{ Topology::Triangle };
				FillMode fillmode{ FillMode::Solid };
				DepthTest depthTest{ DepthTest::On };
				BlendMode blendMode{ BlendMode::AlphaBlend };
				CullMode cullMode{ CullMode::Backface };
            };

            virtual ~PipelineState() = default;
            virtual void Activate() = 0;
        };
    }
}
