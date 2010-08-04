// Copyright (C) 2010 Gabor Papp
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <escheme.h>
#include <iostream>
#include <map>

#include "OpenGL.h"
#include "SHMTexture.h"

using namespace std;

// StartSectionDoc-en
// shm-texture
// The shm-texture module provides functions to use memory areas shared
// by other processes as textures.
// Example:
// (require fluxus-017/shm-texture)
// (clear)
// (define t (shm-texture 5656 320 240 'greyscale))
// (texture t)
// (let ([p (build-plane)]
//       [tcoords (shm-tcoords t)])
//    (with-primitive p
//        (scale #(4 3 1))
//        (pdata-index-map!
//            (lambda (i t)
//                (list-ref tcoords (remainder i 4)))
//            "t")))
// (every-frame
//    (shm-update t))
// EndSectionDoc

static map<int, SHMTexture *> key2shmt;
static map<unsigned, int> id2key;

static SHMTexture *find_texture(int key)
{
	map<int, SHMTexture *>::iterator i = key2shmt.find(key);
	if (i != key2shmt.end())
	{
		return i->second;
	}
	else
	{
		return NULL;
	}
}


// StartFunctionDoc-en
// shm-clear-cache
// Returns: void
// Description:
// The shared memory textures are memory cached. (shm-clear-cache) clears
// the cache, meaning the shared memory will be reattached and the textures
// will be recreated.
// Example:
// (shm-clear-cache)
// EndFunctionDoc

Scheme_Object *shm_clear_cache(int argc, Scheme_Object **argv)
{
	for (map<int, SHMTexture *>::iterator i = key2shmt.begin(); i != key2shmt.end(); ++i)
	{
		delete i->second;
	}
	key2shmt.clear();
	id2key.clear();

	return scheme_void;
}


// StartFunctionDoc-en
// shm-texture key-int width-int height-int format-symbol
// Returns: textureid-number
// Description:
// Makes a texture from a shared memory area key.
// Format specifies the image format 'greyscale or 'rgb.
// Example:
// (define t (shm-texture 5656 256 256 'greyscale))
// (texture t)
// (build-cube)
// (every-frame
//    (shm-update t))
// EndFunctionDoc

Scheme_Object *shm_texture(int argc, Scheme_Object **argv)
{
	MZ_GC_DECL_REG(1);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_REG();

	for (int i = 0; i < 3; i++)
	{
		if (!SCHEME_INTP(argv[i]))
		{
			scheme_wrong_type("shm-texture", "int", i, argc, argv);
		}
	}

	if (!SCHEME_SYMBOLP(argv[3]))
	{
		scheme_wrong_type("shm-texture", "symbol", 3, argc, argv);
	}

	string fmt = scheme_symbol_name(argv[3]);
	int format = -1;
	if (fmt == "rgb")
		format = GL_RGB;
	else
	if (fmt == "greyscale")
		format = GL_LUMINANCE;
	else
	{
		cerr << "shm-texture: unknown format " << fmt << endl;
		MZ_GC_UNREG();
		return scheme_make_integer_value(0);
	}

	int key = SCHEME_INT_VAL(argv[0]);
	int w = SCHEME_INT_VAL(argv[1]);
	int h = SCHEME_INT_VAL(argv[2]);

	SHMTexture *st = find_texture(key);

	unsigned id = 0;
	if (st == NULL)
	{
		try
		{
			st = new SHMTexture(key, w, h, format);
			id = st->get_texture_id();
			key2shmt[key] = st;
			id2key[id] = key;
		}
		catch (SHMTexture::Error e)
		{
			st = NULL;
		}
	}
	else
	{
		id = st->get_texture_id();
	}

	MZ_GC_UNREG();
	return scheme_make_integer_value(id);
}

// StartFunctionDoc-en
// shm-update textureid-number
// Returns: void
// Description:
// Updates a shared memory area texture.
// Example:
// (define t (shm-texture 5656 256 256 'greyscale))
// (texture t)
// (build-cube)
// (every-frame
//    (shm-update t))
// EndFunctionDoc

Scheme_Object *shm_update(int argc, Scheme_Object **argv)
{
	MZ_GC_DECL_REG(1);
	MZ_GC_VAR_IN_REG(0, argv);
	MZ_GC_REG();

	if (!SCHEME_NUMBERP(argv[0]))
		scheme_wrong_type("shm-update", "number", 0, argc, argv);

	unsigned id = (unsigned)scheme_real_to_double(argv[0]);

	// id->key
	map<unsigned, int>::iterator i = id2key.find(id);
	if (i != id2key.end())
	{
		// key->st
		int key = i->second;
		SHMTexture *st = find_texture(key);
		if (st != NULL)
		{
			st->update();
		}
	}

	MZ_GC_UNREG();
	return scheme_void;
}

static Scheme_Object *scheme_vector(float v0, float v1, float v2)
{
    Scheme_Object *ret = NULL;
    Scheme_Object *tmp = NULL;
    MZ_GC_DECL_REG(2);
    MZ_GC_VAR_IN_REG(0, ret);
    MZ_GC_VAR_IN_REG(1, tmp);
    MZ_GC_REG();
    ret = scheme_make_vector(3, scheme_void);
    SCHEME_VEC_ELS(ret)[0] = scheme_make_double(v0);
    SCHEME_VEC_ELS(ret)[1] = scheme_make_double(v1);
    SCHEME_VEC_ELS(ret)[2] = scheme_make_double(v2);

    MZ_GC_UNREG();
    return ret;
}

// StartFunctionDoc-en
// shm-tcoords textureid-number
// Returns: list-of-texture-coordinates
// Description:
// Returns the texture coordinates of the shared memory texture. This is
// necessary, because images are not always non-power-of-two
// textures, while fluxus uses GL_TEXTURE_2D power-of-two textures.
// Example:
// (define t (shm-texture 5656 320 240 'greyscale))
// (texture t)
// (let ([p (build-plane)]
//       [tcoords (shm-tcoords t)])
//    (with-primitive p
//        (scale #(4 3 1))
//        (pdata-index-map!
//            (lambda (i t)
//                (list-ref tcoords (remainder i 4)))
//            "t")))
// (every-frame
//    (shm-update t))
// EndFunctionDoc

Scheme_Object *shm_tcoords(int argc, Scheme_Object **argv)
{
    Scheme_Object *ret = NULL;
    Scheme_Object **coord_list = NULL;
    MZ_GC_DECL_REG(2);
    MZ_GC_VAR_IN_REG(0, argv);
    MZ_GC_VAR_IN_REG(1, coord_list);
    MZ_GC_REG();
    if (!SCHEME_NUMBERP(argv[0]))
        scheme_wrong_type("shm-tcoords", "number", 0, argc, argv);

	SHMTexture *st = NULL;

	// id->key
	unsigned id = (unsigned)scheme_real_to_double(argv[0]);
	map<unsigned, int>::iterator i = id2key.find(id);
	if (i != id2key.end())
	{
		// key->st
		int key = i->second;
		st = find_texture(key);
	}

    if (st != NULL)
    {
        coord_list = (Scheme_Object **)scheme_malloc(4 *
                sizeof(Scheme_Object *));

        float *coords  = st->get_tcoords();

        coord_list[0] = scheme_vector(coords[0], coords[4], coords[2]);
        coord_list[1] = scheme_vector(coords[3], coords[4], coords[5]);
        coord_list[2] = scheme_vector(coords[3], coords[1], coords[5]);
        coord_list[3] = scheme_vector(coords[0], coords[1], coords[2]);

        ret = scheme_build_list(4, coord_list);
    }
    else
    {
        ret = scheme_void;
    }

    MZ_GC_UNREG();
    return ret;
}

Scheme_Object *scheme_reload(Scheme_Env *env)
{
	Scheme_Env *menv = NULL;
	MZ_GC_DECL_REG(2);
	MZ_GC_VAR_IN_REG(0, env);
	MZ_GC_VAR_IN_REG(1, menv);
	MZ_GC_REG();

	// add all the modules from this extension
	menv = scheme_primitive_module(scheme_intern_symbol("shm-texture"), env);

	scheme_add_global("shm-clear-cache", scheme_make_prim_w_arity(shm_clear_cache, "shm-clear-cache", 0, 0), menv);
	scheme_add_global("shm-texture", scheme_make_prim_w_arity(shm_texture, "shm-texture", 4, 4), menv);
	scheme_add_global("shm-update", scheme_make_prim_w_arity(shm_update, "shm-update", 1, 1), menv);
	scheme_add_global("shm-tcoords", scheme_make_prim_w_arity(shm_tcoords, "shm-tcoords", 1, 1), menv);

	scheme_finish_primitive_module(menv);
	MZ_GC_UNREG();

	return scheme_void;
}

Scheme_Object *scheme_initialize(Scheme_Env *env)
{
	return scheme_reload(env);
}

Scheme_Object *scheme_module_name()
{
	return scheme_intern_symbol("shm-texture");
}

