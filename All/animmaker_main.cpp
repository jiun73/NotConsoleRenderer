#include "GLUU.h"
#include "AnimationX.h"


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

namespace ANIMMAKER
{
	void animmaker_main() 
	{
		GLUU::import_function<vector<string>()>("$file", []()
			{
				return open_dialog();
			});

		GLUU::import_function<SDL_Texture* (const string&)>(":get_tex", get_sdl_texture);

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