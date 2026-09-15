#include "sdl/SdlTextRenderer.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <array>
#include <iostream>
#include <string>
#include <utility>

#include "sdl/SdlPaths.h"

namespace neon::sdl {

	struct SdlTextRenderer::Impl {
		TTF_Font* titleFont = nullptr;
		TTF_Font* bodyFont = nullptr;
		bool ownsTtfInitialization = false;
		bool chineseFont = false;

		~Impl() {
			if (titleFont != nullptr) {
				TTF_CloseFont(titleFont);
			}

			if (bodyFont != nullptr) {
				TTF_CloseFont(bodyFont);
			}

			if (ownsTtfInitialization) {
				TTF_Quit();
			}
		}
	};

	namespace {

		struct FontCandidate {
			std::string path;
			bool supportsChinese = false;
		};

		std::array<FontCandidate, 5> fontCandidates() {
			return {
				FontCandidate{
					assetFilePath("fonts/NeonSiege.ttf"),
					true
				},
				FontCandidate{
					"C:\\Windows\\Fonts\\msyh.ttc",
					true
				},
				FontCandidate{
					"C:\\Windows\\Fonts\\simhei.ttf",
					true
				},
				FontCandidate{
					"C:\\Windows\\Fonts\\simsun.ttc",
					true
				},
				FontCandidate{
					"C:\\Windows\\Fonts\\arial.ttf",
					false
				}
			};
		}

	}//namespace

	SdlTextRenderer::SdlTextRenderer()
		: impl_(std::make_unique<Impl>()) {

		if (TTF_WasInit() == 0) {
			if (TTF_Init() != 0) {
				std::cerr <<
					"TTF_Init failed: " <<
					TTF_GetError() << '\n';
				return;
			}

			impl_->ownsTtfInitialization = true;
		}

		bool opened = false;

		for (const FontCandidate& candidate :
			fontCandidates()) {

			if (candidate.path.empty()) {
				continue;
			}

			TTF_Font* titleFont =
				TTF_OpenFont(candidate.path.c_str(), 32);
			TTF_Font* bodyFont =
				TTF_OpenFont(candidate.path.c_str(), 22);

			if (titleFont == nullptr || bodyFont == nullptr) {
				if (titleFont != nullptr) {
					TTF_CloseFont(titleFont);
				}

				if (bodyFont != nullptr) {
					TTF_CloseFont(bodyFont);
				}

				continue;
			}

			impl_->titleFont = titleFont;
			impl_->bodyFont = bodyFont;
			impl_->chineseFont = candidate.supportsChinese;
			opened = true;
			break;
		}

		if (!opened) {
			std::cerr <<
				"No usable font found; game-over text will be hidden.\n";
		}
	}

	SdlTextRenderer::~SdlTextRenderer() = default;

	SdlTextRenderer::SdlTextRenderer(
		SdlTextRenderer&& other) noexcept = default;

	SdlTextRenderer& SdlTextRenderer::operator=(
		SdlTextRenderer&& other) noexcept = default;

	bool SdlTextRenderer::ready() const noexcept {
		return impl_ != nullptr &&
			impl_->titleFont != nullptr &&
			impl_->bodyFont != nullptr;
	}

	bool SdlTextRenderer::supportsChinese() const noexcept {
		return ready() && impl_->chineseFont;
	}

	bool SdlTextRenderer::drawCentered(
		SDL_Renderer* renderer,
		std::string_view text,
		TextStyle style,
		int centerX,
		int centerY,
		const SDL_Color& color) const {

		return drawAligned(
			renderer,
			text,
			style,
			centerX,
			centerY,
			TextAlignment::Center,
			color
		);
	}

	bool SdlTextRenderer::drawLeft(
		SDL_Renderer* renderer,
		std::string_view text,
		TextStyle style,
		int leftX,
		int centerY,
		const SDL_Color& color) const {

		return drawAligned(
			renderer,
			text,
			style,
			leftX,
			centerY,
			TextAlignment::Left,
			color
		);
	}

	bool SdlTextRenderer::drawRight(
		SDL_Renderer* renderer,
		std::string_view text,
		TextStyle style,
		int rightX,
		int centerY,
		const SDL_Color& color) const {

		return drawAligned(
			renderer,
			text,
			style,
			rightX,
			centerY,
			TextAlignment::Right,
			color
		);
	}

	bool SdlTextRenderer::drawAligned(
		SDL_Renderer* renderer,
		std::string_view text,
		TextStyle style,
		int anchorX,
		int centerY,
		TextAlignment alignment,
		const SDL_Color& color) const {

		if (!ready() || renderer == nullptr || text.empty()) {
			return false;
		}

		TTF_Font* font =
			style == TextStyle::Title
			? impl_->titleFont
			: impl_->bodyFont;

		if (font == nullptr) {
			return false;
		}

		const std::string textString{text};
		SDL_Surface* surface =
			TTF_RenderUTF8_Blended(
				font,
				textString.c_str(),
				color
			);

		if (surface == nullptr) {
			return false;
		}

		SDL_Texture* texture =
			SDL_CreateTextureFromSurface(renderer, surface);

		if (texture == nullptr) {
			SDL_FreeSurface(surface);
			return false;
		}

		int destinationX = anchorX;
		if (alignment == TextAlignment::Center) {
			destinationX -= surface->w / 2;
		}
		else if (alignment == TextAlignment::Right) {
			destinationX -= surface->w;
		}

		const SDL_Rect destination{
			destinationX,
			centerY - surface->h / 2,
			surface->w,
			surface->h
		};

		const int result =
			SDL_RenderCopy(renderer, texture, nullptr, &destination);

		SDL_DestroyTexture(texture);
		SDL_FreeSurface(surface);
		return result == 0;
	}

}//namespace neon::sdl
