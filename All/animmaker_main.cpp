#include "pch.h"
#include "AnimationX.h"
#include "EditorPlane.h"

#define __IMPORT_MAKE_REF__(n) if (str == #n) return make_generic_ref(obj.##n)


const FactoryManagerAdder<std::pair< string, FrameXColliderSet>>* vectosr_anfdsfdsimf__adder = new FactoryManagerAdder<std::pair< string, FrameXColliderSet>>("pair(string-FrameXColliderSet)", true);
GLUU::ImportInspector<std::pair< string, FrameXColliderSet>> animf2_inpector([](shared_generic gen, const string& str) -> shared_generic
	{
		std::pair< string, FrameXColliderSet>& obj = *(std::pair< string, FrameXColliderSet>*)(gen->raw_bytes());
		if (str == "first") return make_generic_ref(obj.first);
		if (str == "second") return make_generic_ref(obj.second);
		return nullptr;
	});

__REGISTER_CLASS__(AnimationXColliders);
GLUU::ImportInspector<AnimationXColliders> animcol_inpector([](shared_generic gen, const string& str) -> shared_generic
	{
		AnimationXColliders& obj = *(AnimationXColliders*)(gen->raw_bytes());
		if (str == "tags") return make_generic_container_ref(obj.tags);
		return nullptr;
	});

const FactoryManagerAdder<vector<FrameXColliderSet>>* vector_animgfcol__adder = new FactoryManagerAdder<vector<FrameXColliderSet>>("vector(FrameXColliderSet)", true);
__REGISTER_CLASS__(FrameXColliderSet);
GLUU::ImportInspector<FrameXColliderSet> animfcolset_inpector([](shared_generic gen, const string& str) -> shared_generic
	{
		FrameXColliderSet& obj = *(FrameXColliderSet*)(gen->raw_bytes());
		if (str == "colliders") return make_generic_container_ref(obj.colliders);
		return nullptr;
	});

__REGISTER_CLASS__(FrameXCollider);
GLUU::ImportInspector<FrameXCollider> animfcol_inpector([](shared_generic gen, const string& str) -> shared_generic
	{
		FrameXCollider& obj = *(FrameXCollider*)(gen->raw_bytes());
		if (str == "anchor") return make_generic_ref(obj.anchor);
		if (str == "bounds") return make_generic_ref(obj.bounds);
		return nullptr;
	});

const FactoryManagerAdder<vector<AnimationXColliders>>* vector_animcol__adder = new FactoryManagerAdder<vector<AnimationXColliders>>("vector(AnimationXColliders)", true);

__REGISTER_CLASS__(AnimationX);
GLUU::ImportInspector<AnimationX> anim_inpector([](shared_generic gen, const string& str) -> shared_generic
	{
		AnimationX& obj = *(AnimationX*)(gen->raw_bytes());
		if (str == "frames") return make_generic_container_ref(obj.frames);
		if (str == "textures") return make_generic_container_ref(obj.textures);
		if (str == "name") return make_generic_ref(obj.name);
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
		if (str == "anchors") return make_generic_container_ref(obj.anchors);
		//if (str == "colliders") return make_generic_ref(obj.colliders);
		if (str == "full_source") return make_generic_ref(obj.full_source);
		if (str == "origin") return make_generic_ref(obj.origin);
		if (str == "source") return make_generic_ref(obj.source);
		return nullptr;
	});

const FactoryManagerAdder<std::pair< string, V2d_i>>* vectosr_animf__adder = new FactoryManagerAdder<std::pair< string, V2d_i>>("pair(string-V2d_i)", true);
GLUU::ImportInspector<std::pair< string, V2d_i>> gfdgdanimf2_inpector([](shared_generic gen, const string& str) -> shared_generic
	{
		std::pair< string, V2d_i>& obj = *(std::pair< string, V2d_i>*)(gen->raw_bytes());
		if (str == "first") return make_generic_ref(obj.first);
		if (str == "second") return make_generic_ref(obj.second);
		return nullptr;
	});

__REGISTER_CLASS__(V2d_i);
GLUU::ImportInspector<V2d_i> animf22_inpector([](shared_generic gen, const string& str) -> shared_generic
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
	GLUU::SeqVar <int> current_anchor;

	EditorPlane plane;

	GLUU_Make(3, "FRAME_CREATOR")
	{
		auto ptr = make_shared<Custom_FrameCreatorWidget>();

		ptr->animation.set(args.at(0), parser);
		ptr->current_frame.set(args.at(1), parser);
		ptr->current_anchor.set(args.at(2), parser);

		return ptr;
	}

	void draw_anchor(V2d_i pos, Color col, V2d_i img_pos)
	{
		V2d_i origin = ((V2d_d)(pos) * plane.scale) + (V2d_d)img_pos;
		V2d_i origin_vt = { origin.x, origin.y - 5 };
		V2d_i origin_vb = { origin.x, origin.y + 5 };
		V2d_i origin_ht = { origin.x - 5, origin.y };
		V2d_i origin_hb = { origin.x + 5, origin.y };

		pencil(COLOR_BLACK);
		draw_line(origin_vt + 1, origin_vb + 1);
		draw_line(origin_ht + 1, origin_hb + 1);

		pencil(col);
		draw_line(origin_vt, origin_vb);
		draw_line(origin_ht, origin_hb);
	}


	void update(GLUU::Element& graphic, GLUU::MouseInfo& mouse) override
	{
		plane.dest = graphic.last_dest;
		plane.draw_checkered_background();
		plane.update();

		if (plane.is_pressed())
		{
			if(current_anchor == -1)
				current_frame().origin = (V2d_d)plane.deproject(mouse_position()) / plane.scale;
			else
			{
				auto it = current_frame().anchors.begin();
				std::advance(it, (size_t)current_anchor);
				it->second = (V2d_d)plane.deproject(mouse_position()) / plane.scale;
			}
		}

		SDL_Texture* tex = animation().textures.at(current_frame().tex);
		SDL_RenderSetClipRect(get_sdl_ren(), graphic.last_dest.SDL());
		int x, y;
		SDL_QueryTexture(tex, NULL, NULL, &x, &y);
		Rect image = plane.project({ 0, {x,y} });
		SDL_RenderCopy(get_sdl_ren(), tex, NULL, image.SDL());

		draw_anchor(current_frame().origin, COLOR_PINK, image.pos);

		for (auto& p : current_frame().anchors)
		{
			draw_anchor(p.second, COLOR_GREEN, image.pos);
		}

		
		SDL_RenderSetClipRect(get_sdl_ren(), NULL);
		
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

	void update(GLUU::Element& graphic, GLUU::MouseInfo& mouse) override
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
		GLUU::import_function<vector<string>()>("$files", []()
			{
				return open_dialog();
			});

		GLUU::import_function<string()>("$file", []()
			{
				return open_dialog_single();
			});

		GLUU::import_function<string()>("$new_file", []()
			{
				return open_dialog_new_file();
			});

		GLUU::import_function<SDL_Texture* (const string&)>(":get_tex", get_sdl_texture);
		GLUU::import_function<V2d_i(SDL_Texture*)>(":get_tex_sz", [](SDL_Texture* tex) -> V2d_i
			{
				int x, y;
				SDL_QueryTexture(tex, NULL, NULL, &x, &y);
				return { x,y };
			});

		GLUU::import_function<void(vector<AnimationX>& , string)>(":write_anim_to_file", [](vector<AnimationX>& anim, string s)
			{
				NCR::File file(s, NCR::Files::FILE_WRITING);
				for (auto& a : anim)
				{
					std::cout << "writing '" << a.name << "'" << std::endl;
					a.readwrite(file, "");
				}
			});

		GLUU::import_function<vector<AnimationX>(string)>(":read_anim_from_file", [](string s)
			{
				NCR::File file(s, NCR::Files::FILE_READING);

				vector<AnimationX> ret;
				for (auto& a : file("data").get_chunks())
				{
					AnimationX new_anim;
					std::cout << "reading '" << a << "' from " << s << std::endl;
					new_anim.readwrite(file, a);
					ret.push_back(new_anim);
				}
				return ret;
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