/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "Common/Graphics/RendererBase.h"
#include "Common/Math/Maths.h"
#include "Common/NCLAliases.h"
#include <string>
#include <vector>
#include <span>
#include <functional>

#ifdef _WIN32
#include "windows.h"
#endif

#ifdef _DEBUG
#define OPENGL_DEBUGGING
#endif

namespace NCL {
	class MeshGeometry;

	namespace Maths {
		class Matrix4;
	}

	namespace Rendering {
		class ShaderBase;
		class TextureBase;

		class OGLMesh;
		class OGLShader;
		class OGLTexture;

		class SimpleFont;

		// TODO: Sort out what we're doing with these.
		struct RenderColourAttachment {
			std::reference_wrapper<const OGLTexture> texture;
		};

		struct RenderDepthStencilAttachment {
			std::reference_wrapper<const OGLTexture> texture;
		};

		struct RenderInfo {
			std::string_view name;
			std::span<const RenderColourAttachment> colorAttachments;
			std::optional<RenderDepthStencilAttachment> depthAttachment = {};
			std::optional<RenderDepthStencilAttachment> stencilAttachment = {};
		};

		class OGLRenderer : public RendererBase
		{

		public:
			friend class OGLRenderer;
			OGLRenderer(Window& w);
			~OGLRenderer();

			void OnWindowResize(int w, int h)	override;
			bool HasInitialised()				const override {
				return initState;
			}

			void ForceValidDebugState(bool newState) {
				forceValidDebugState = newState;
			}

			virtual bool SetVerticalSync(VerticalSyncState s);

			Vector2 GetDimensions() const { return Vector2(currentWidth, currentHeight); }

			void DrawString(const std::string& text, const Vector2&pos, const Vector4& colour = Vector4(0.75f, 0.75f, 0.75f,1), float size = 20.0f );
			void DrawLine(const Vector3& start, const Vector3& end, const Vector4& colour);

			virtual Matrix4 SetupDebugLineMatrix()	const;
			virtual Matrix4 SetupDebugStringMatrix()const;

		protected:			
			void BeginFrame()	override;
			void RenderFrame()	override;
			void EndFrame()		override;
			void SwapBuffers()  override;

			void DrawDebugData();
			void DrawDebugStrings();
			void DrawDebugLines();

			void BindShader(ShaderBase*s);
			void BindTextureToShader(const TextureBase*t, const std::string& uniform, int texUnit) const;
			void BindMesh(MeshGeometry*m);
			void DrawBoundMesh(int subLayer = 0, int numInstances = 1);
#ifdef _WIN32
			void InitWithWin32(Window& w);
			void DestroyWithWin32();
			HDC		deviceContext;		//...Device context?
			HGLRC	renderContext;		//Permanent Rendering Context		
#endif
		protected:
			OGLMesh* boundMesh;
			OGLShader* boundShader;
		private:
			struct DebugString {
				Maths::Vector4 colour;
				Maths::Vector2	pos;
				float			size;
				std::string		text;
			};

			struct DebugLine {
				Maths::Vector3 start;
				Maths::Vector3 end;
				Maths::Vector4 colour;
			};

			OGLMesh* debugLinesMesh;
			OGLMesh* debugTextMesh;

			OGLShader*  debugShader;
			SimpleFont* font;
			std::vector<DebugString>	debugStrings;
			std::vector<DebugLine>		debugLines;

			bool initState;
			bool forceValidDebugState;
		};

		namespace Cmds {
			void BindTexture(uint unit, uint texture);
			// True = repeating, false = clamp
			void SetTextureRepeating(uint target, bool state);
			// True = linear filtering, False = nearest filtering,
			void SetTextureFiltering(uint target, bool state);

			// Utils
			bool IsShaderBound(int programID);
			bool IsTextureBound(int texID);
			int GetBoundShader();	
		}
	}
}