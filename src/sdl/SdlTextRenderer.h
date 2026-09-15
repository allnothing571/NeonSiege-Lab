#pragma once

#include <memory>
#include <string_view>

struct SDL_Color;
struct SDL_Renderer;

namespace neon::sdl {

	enum class TextStyle {
		Title,
		Body
	};

	enum class TextAlignment {
		Left,
		Center,
		Right
	};

	class SdlTextRenderer {
	public:
		SdlTextRenderer();
		~SdlTextRenderer();

		SdlTextRenderer(const SdlTextRenderer&) = delete;
		SdlTextRenderer& operator=(
			const SdlTextRenderer&) = delete;

		SdlTextRenderer(SdlTextRenderer&&) noexcept;
		SdlTextRenderer& operator=(
			SdlTextRenderer&&) noexcept;

		bool ready() const noexcept;
		bool supportsChinese() const noexcept;

		bool drawCentered(
			SDL_Renderer* renderer,
			std::string_view text,
			TextStyle style,
			int centerX,
			int centerY,
			const SDL_Color& color
		) const;

		bool drawLeft(
			SDL_Renderer* renderer,
			std::string_view text,
			TextStyle style,
			int leftX,
			int centerY,
			const SDL_Color& color
		) const;

		bool drawRight(
			SDL_Renderer* renderer,
			std::string_view text,
			TextStyle style,
			int rightX,
			int centerY,
			const SDL_Color& color
		) const;

	private:
		bool drawAligned(
			SDL_Renderer* renderer,
			std::string_view text,
			TextStyle style,
			int anchorX,
			int centerY,
			TextAlignment alignment,
			const SDL_Color& color
		) const;

		struct Impl;
		std::unique_ptr<Impl> impl_;
	};

}//namespace neon::sdl
