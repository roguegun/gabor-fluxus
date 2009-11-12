/*
 * Copyright (C) 2009 Gabor Papp
 * http://mndl.hu/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Some part of the code is borrowed from Pete Warden's CoreImage FF1.0 source.
 * http://www.petewarden.com/FFCoreImage_v0-02.zip
 */

#import <stdio.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <QuartzCore/CIFilter.h>
#import <QuartzCore/CIContext.h>
#import <QuartzCore/CIVector.h>

#import "CIObjC.h"

#define OBJC_FILTER_NAME @FILTER_NAME
#define OBJC_FILTER_ID @FILTER_ID

enum {
	FF_TYPE_RED = 2,
	FF_TYPE_GREEN,
	FF_TYPE_BLUE,
	FF_TYPE_XPOS,
	FF_TYPE_YPOS,
	FF_TYPE_ALPHA, /* not a standard FF type */
	FF_TYPE_STANDARD = 10
	/* FF_TYPE_TEXT = 100 */
};

typedef struct {
	float def;
	float min;
	float max;
	float value;
	char name[16];
	NSString *key;
	int type;
} ParameterDefault;

typedef struct {
	NSString *key;
} Image;

#define MAX_PARAMETERS 32
#define MAX_INPUT_IMAGES 32

static int parameter_count;
static ParameterDefault parameters[MAX_PARAMETERS];

static int image_count;
static Image images[MAX_INPUT_IMAGES];

static void ci_init_parameters(void)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	CIFilter *filter = [CIFilter filterWithName: OBJC_FILTER_NAME];
	NSDictionary *filter_attributes = [filter attributes];
	NSArray *input_keys = [filter inputKeys];

	const int input_key_count = [input_keys count];

	parameter_count = 0;
	image_count = 0;

	/* reads all parameters and input images, sets minimum, maximum, default values */
	int i;
	for (i = 0; i < input_key_count; i++)
	{
		NSString *current_input_key = [input_keys objectAtIndex: i];
		NSDictionary *current_input_attributes = [filter_attributes objectForKey: current_input_key];

		NSString *input_key_class = [current_input_attributes objectForKey: kCIAttributeClass];
		NSString *input_key_name = [current_input_attributes objectForKey: kCIAttributeDisplayName];
		NSData *input_key_name_data = [input_key_name dataUsingEncoding: NSASCIIStringEncoding
			allowLossyConversion: YES];

		char name[16];
		int name_length = [input_key_name_data length];
		if (name_length > 15)
			name_length = 15;

		memcpy(name, [input_key_name_data bytes], name_length);
		name[name_length] = 0;
		if ((name_length > 1) && (name[name_length - 1] == ' ')) /* delete trailing space */
			name[name_length - 1] = 0;

		if (([input_key_class isEqual:@"NSNumber"]) && (parameter_count < MAX_PARAMETERS))
		{
			ParameterDefault *p = &parameters[parameter_count];
			p->value = p->def = 0.5;
			p->min = 0.0;
			p->max = 1.0;
			memcpy(p->name, name, 16);
			p->type = FF_TYPE_STANDARD;

			[current_input_key retain];
			p->key = current_input_key;

			NSNumber *slider_min = [current_input_attributes objectForKey: kCIAttributeSliderMin];
			NSNumber *slider_max = [current_input_attributes objectForKey: kCIAttributeSliderMax];
			NSNumber *slider_def = [current_input_attributes objectForKey: kCIAttributeDefault];

			if (slider_min != nil)
			{
				p->min = [slider_min floatValue];
			}
			if (slider_max != nil)
			{
				p->max = [slider_max floatValue];
			}
			if (slider_def != nil)
			{
				p->value = p->def = [slider_def floatValue];
			}

			parameter_count++;
		}
		else
		if (([input_key_class isEqual:@"CIVector"]) && (parameter_count < (MAX_PARAMETERS - 1)))
		{
			ParameterDefault *p = &parameters[parameter_count];
			p[0].value = p[0].def = 0.5; /* scaled with input texture width */
			p[0].min = 0.0;
			p[0].max = 1.0;
			if (strlen(name) > 13)
				name[13] = 0;
			snprintf(p[0].name, 15, "%s-X", name);
			p[0].type = FF_TYPE_XPOS;

			p[1].value = p[1].def = 0.5; /* scaled with input texture height */
			p[1].min = 0.0;
			p[1].max = 1.0;
			snprintf(p[1].name, 15, "%s-Y", name);
			p[1].type = FF_TYPE_YPOS;

			[current_input_key retain];
			p[0].key = current_input_key;
			p[1].key = current_input_key;

			/*
			CIVector *slider_min = [current_input_attributes objectForKey: kCIAttributeSliderMin];
			CIVector *slider_max = [current_input_attributes objectForKey: kCIAttributeSliderMax];
			CIVector *slider_def = [current_input_attributes objectForKey: kCIAttributeDefault];

			if (slider_def != nil)
			{
				p[0].value = p[0].def = [slider_def X];
				p[1].value = p[1].def = [slider_def Y];
			}

			if (slider_min != nil)
			{
				p[0].min = [slider_min X];
				p[1].min = [slider_min Y];
			}

			if (slider_max != nil)
			{
				p[0].max = [slider_max X];
				p[1].max = [slider_max Y];
			}
			else
			{
				p[0].max = p[0].def * 2;
				p[1].max = p[1].def * 2;
			}
			*/

			parameter_count += 2;
		}
		else
		if (([input_key_class isEqual:@"CIColor"]) && (parameter_count < (MAX_PARAMETERS - 3)))
		{
			ParameterDefault *p = &parameters[parameter_count];

			[current_input_key retain];
			if (strlen(name) > 13)
				name[13] = 0;
			int types[] = { FF_TYPE_RED, FF_TYPE_GREEN, FF_TYPE_BLUE, FF_TYPE_ALPHA };
			char *letters = "RGBA";
			for (int j = 0; j < 4; j++)
			{
				p[j].value = p[j].def = 0.5;
				p[j].min = 0.0;
				p[j].max = 1.0;
				snprintf(p[j].name, 15, "%s-%c", name, letters[j]);
				p[j].type = types[j];
				p[j].key = current_input_key;
			}

			CIColor *slider_min = [current_input_attributes objectForKey: kCIAttributeSliderMin];
			CIColor *slider_max = [current_input_attributes objectForKey: kCIAttributeSliderMax];
			CIColor *slider_def = [current_input_attributes objectForKey: kCIAttributeDefault];

			if (slider_def != nil)
			{
				p[0].value = p[0].def = [slider_def red];
				p[1].value = p[1].def = [slider_def green];
				p[2].value = p[2].def = [slider_def blue];
				p[3].value = p[3].def = [slider_def alpha];
			}
			if (slider_min != nil)
			{
				p[0].min = [slider_min red];
				p[1].min = [slider_min green];
				p[2].min = [slider_min blue];
				p[3].min = [slider_min alpha];
			}
			if (slider_max != nil)
			{
				p[0].max = [slider_max red];
				p[1].max = [slider_max green];
				p[2].max = [slider_max blue];
				p[3].max = [slider_max alpha];
			}

			parameter_count += 4;
		}
		else
		if (([input_key_class isEqual:@"CIImage"]) && (image_count < MAX_INPUT_IMAGES))
		{
			[current_input_key retain];
			images[image_count].key = current_input_key;
			image_count++;
		}
		else
		{
			NSLog(@"Unknown param class:%@", input_key_class);
		}
		/* TODO: NSAffineTransform CIAffineTransform
		 * CIVector - CIColorMatrix
		 * NSData - CIColorCube */

	}

	/*
	printf("params %d\n", parameter_count);
	for (int i = 0; i < parameter_count; i++)
	{
		printf("%d %s def: %f min: %f max: %f\n", i, parameters[i].name, parameters[i].def, parameters[i].min,
												parameters[i].max);
	}
	*/

	[pool release];
}

void ci_init()
{
	ci_init_parameters();
}

static char filter_name[17];

char *ci_get_name()
{
	const char *str = [OBJC_FILTER_NAME UTF8String];
	strncpy(filter_name, (char *)str, 16);
	return filter_name;
}

static char filter_id[5];

char *ci_get_id(void)
{
	const char *str = [OBJC_FILTER_ID UTF8String];
	strncpy(filter_id, (char *)str, 4);
	return filter_id;
}

int ci_get_parameter_count(void)
{
	return parameter_count;
}

int ci_get_input_frame_count(void)
{
	return image_count;
}

void ci_set_parameter(int i, float v)
{
	parameters[i].value = v;
}

void ci_get_parameter_defaults(int i, char **pname, float *def, float *min, float *max, int *type)
{
	*pname = parameters[i].name;
	*def = parameters[i].def;
	*min = parameters[i].min;
	*max = parameters[i].max;
	*type = parameters[i].type;
	if (*type == FF_TYPE_ALPHA) /* not a standard FF type */
		*type = FF_TYPE_STANDARD;
}

void ci_process(unsigned *handles, int width, int height)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	CIContext *ci_context;

	const NSOpenGLPixelFormatAttribute attr[] = {
		NSOpenGLPFAAllRenderers,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAColorSize, 32,
		0
	};

	NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:(void *)&attr];

	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();

	ci_context = [CIContext contextWithCGLContext: CGLGetCurrentContext()
		pixelFormat: [pf CGLPixelFormatObj]
		options: nil];


	CIFilter *filter = [CIFilter filterWithName: OBJC_FILTER_NAME];
	[filter setDefaults];

	CGSize size = { width, height };

	/* set input images */
	for (int i = 0; i < image_count; i++)
	{
		CIImage *source = [CIImage imageWithTexture:handles[i] size:size flipped:NO colorSpace:cs];
		[filter setValue:source forKey: images[i].key];
	}

	/* send current parameter values */
	for (int i = 0; i < parameter_count; i++)
	{
		switch (parameters[i].type)
		{
			case FF_TYPE_STANDARD:
				[filter setValue:[NSNumber numberWithFloat: parameters[i].value]
					forKey: parameters[i].key];
				break;

			case FF_TYPE_XPOS:
				[filter setValue:[NSNumber numberWithFloat: parameters[i].value]
					forKey: parameters[i].key];
				[filter setValue:[CIVector vectorWithX: width * parameters[i].value
							   Y: height * parameters[i + 1].value] forKey: parameters[i].key];
				i++;
				break;

			case FF_TYPE_RED:
				[filter setValue:[CIColor colorWithRed: parameters[i].value
							green: parameters[i + 1].value
							blue: parameters[i + 2].value
							alpha: parameters[i + 3].value]
							forKey: parameters[i].key];
				i += 3;
				break;
		}
	}

	/* render the output */
	CIImage *result = [filter valueForKey:@"outputImage"];

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
	[ci_context drawImage:result atPoint:CGPointZero fromRect:CGRectMake(0, 0, width, height)];
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	CGColorSpaceRelease(cs);

	[pool release];
}

