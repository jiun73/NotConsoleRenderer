#include "pch.h"
#include "AnimationX.h"
#include "EditorPlane.h"

#define __IMPORT_MAKE_REF__(n) if (str == #n) return make_generic_ref(obj.##n)


__REGISTER_CLASS__(AnimationX);
GLUU::ImportInspector<AnimationX> anim_inpector([](shared_generic gen, const string& str) -> shared_generic
	{
		AnimationX& obj = *(AnimationX*)(gen->raw_bytes());
		if (str == "frames") return make_generic_container_ref(obj.frames);
		if (str == "textures") return make_generic_container_ref(obj.textures);
		return nullptr;
	});

const FactoryManagerAdder<vector<AnimationX>>* vector_anim__adder = new FactoryManagerAdder<vector<AnimationX>>("vector(AnimationX)", true);
const FactoryManagerAdder<vector<AnimationFrameX>>* vector_animf__adder = new FactoryManagerAdder<vector<AnimationFrameX>>("vector(AnimationFrameX)", true);

__REGISTER_CLASS__(AnimationFrameX);
GLUU::ImportInspector<AnimationFrameX> animf_inpector([](shared_generic gen, const string& str) -> shared_generic
	{
		AnimationFrameX& obj = *(AnimationFrameX*)(gen->raw_bytes());
		if (str == "time") return make_generic_ref(obj.time);
		if (str == "tex") return make_generic_ref(obj.tex);
		if (str == "anchors") return make_generic_ref(obj.anchors);
		if (str == "colliders") return make_generic_ref(obj.colliders);
		if (str == "full_source") return make_generic_ref(obj.full_source);
		if (str == "origin") return make_generic_ref(obj.origin);
		if (str == "source") return make_generic_ref(obj.source);
		return nullptr;
	});

__REGISTER_CLASS__(V2d_i);
GLUU::ImportInspector<V2d_i> animf2_inpector([](shared_generic gen, const string& str) -> shared_generic
	{
		V2d_i& obj = *(V2d_i*)(gen->raw_bytes());
		if (str == "x") return make_generic_ref(obj.x);
		if (str == "y") return make_generic_ref(obj.y);
		return nullptr;
	});

__REGISTER_CLASS__(Rect);
GLUU::ImportInspector<Rect> animf3_inpector([](shared_generic gen, const string& str) -> shared_generic
	{
		Rect& obj = *(Rect*)(gen->raw_bytes());
		if (str == "pos") return make_generic_ref(obj.pos);
		if (str == "sz") return make_generic_ref(obj.sz);
		return nullptr;
	});

class Custom_FrameCreatorWidget : public GLUU::Widget
{
	GLUU::SeqVar<AnimationX> animation;
	GLUU::SeqVar<AnimationFrameX> current_frame;

	EditorPlane plane;

	GLUU_Make(2, "FRAME_CREATOR")
	{
		auto ptr = make_shared<Custom_FrameCreatorWidget>();

		ptr->animation.set(args.at(0), parser);
		ptr->current_frame.set(args.at(1), parser);

		return ptr;
	}

	void update(GLUU::Element& graphic) override
	{
		plane.dest = graphic.last_dest;
		plane.draw_checkered_background();
		plane.update();

		if (plane.is_pressed())
		{
			current_frame().origin = plane.deproject(mouse_position());
		}

		SDL_Texture* tex = animation().textures.at(current_frame().tex);
		SDL_RenderSetClipRect(get_sdl_ren(), graphic.last_dest.SDL());
		int x, y;
		SDL_QueryTexture(tex, NULL, NULL, &x, &y);
		Rect image = plane.project({ 0, {x,y} });

		SDL_RenderCopy(get_sdl_ren(), tex, NULL, image.SDL());
		SDL_RenderSetClipRect(get_sdl_ren(), NULL);
		V2d_i origin = ((V2d_d)(current_frame().origin) * plane.scale) + (V2d_d)image.pos;
		V2d_i origin_vt = { origin.x, origin.y - 5 };
		V2d_i origin_vb = { origin.x, origin.y + 5 };
		V2d_i origin_ht = { origin.x - 5, origin.y };
		V2d_i origin_hb = { origin.x + 5, origin.y };

		pencil(COLOR_BLACK);
		draw_line(origin_vt + 1, origin_vb + 1);
		draw_line(origin_ht + 1, origin_hb + 1);

		pencil(COLOR_PINK);
		draw_line(origin_vt, origin_vb);
		draw_line(origin_ht, origin_hb);
	}
};

class Custom_AnimationPrewiewWidget : public GLUU::Widget
{
	GLUU::SeqVar<AnimationX> animation;

	V2d_d pos = 0;
	double scale = 1;

	GLUU_Make(1, "ANIMATION_PREVIEW")
	{
		auto ptr = make_shared<Custom_AnimationPrewiewWidget>();

		ptr->animation.set(args.at(0), parser);

		return ptr;
	}

	void update(GLUU::Element& graphic) override
	{
		double maxscalex = graphic.last_dest.sz.x / (double)animation().max_animation_size().x;
		double maxscaley = graphic.last_dest.sz.y / (double)animation().max_animation_size().y;

		animation().scale = std::min(maxscalex, maxscaley);
		animation().position = graphic.last_dest.pos;
		animation().render(get_sdl_ren(), true);
	}
};

inline GLUU::ImportWidget<Custom_FrameCreatorWidget> imported_widget;
inline GLUU::ImportWidget<Custom_AnimationPrewiewWidget> imported_widget2;

namespace ANIMMAKER
{
	void animmaker_main() 
	{
		GLUU::import_function<vector<string>()>("$file", []()
			{
				return open_dialog();
			});

		GLUU::import_function<SDL_Texture* (const string&)>(":get_tex", get_sdl_texture);
		GLUU::import_function<V2d_i(SDL_Texture*)>(":get_tex_sz", [](SDL_Texture* tex) -> V2d_i
			{
				int x, y;
				SDL_QueryTexture(tex, NULL, NULL, &x, &y);
				return { x,y };
			});

		GLUU::Compiled_ptr menu = GLUU::parse_file("AnimMaker.gluu");

		while (run())
		{
			pencil(COLOR_CYAN);
			draw_clear();

			menu->render({ 0,get_logical_size() });
		}
	}

	GLUU_IMPORT_MAIN(animmaker_main);
}