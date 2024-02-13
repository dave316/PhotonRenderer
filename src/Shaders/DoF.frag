#version 450 core

uniform sampler2D bgTex;
uniform sampler2D renderTex;
uniform sampler2D depthTex;
smooth in vec2 texCoord;

layout(location = 0) out vec4 outColor;

#define PI  3.14159265

uniform float width;
uniform float height;
uniform vec2 texel;

uniform float focalDepth;
uniform float focalLength = 4.3;
uniform float fstop = 1.5;

uniform float znear = 0.1;
uniform float zfar = 100.0;

//------------------------------------------
//user variables

int samples = 25; //samples on the first ring
int rings = 5; //ring count

float CoC = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)

bool autofocus = false; //use autofocus in shader? disable if you use external focalDepth value
vec2 focus = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)
float maxblur = 1.0; //clamp value of max blur (0.0 = no blur,1.0 default)

float threshold = 1.0; //highlight threshold;
float gain = 0.0; //highlight gain;

float bias = 0.5; //bokeh edge bias
float fringe = 0.7f; //bokeh chromatic aberration/fringing

bool noise = true; //use noise instead of pattern for sample dithering
float namount = 0.0001; //dither amount

uniform float a1, a2, a3;
uniform float b1, b2, b3;
uniform float c1, c2, c3;

vec3 color(vec2 coords,float blur) //processing the sample
{
	vec3 col = vec3(0.0);

	col.r = texture2D(renderTex,coords + vec2(0.0,1.0)*texel*fringe*blur).r;
	col.g = texture2D(renderTex,coords + vec2(-0.866,-0.5)*texel*fringe*blur).g;
	col.b = texture2D(renderTex,coords + vec2(0.866,-0.5)*texel*fringe*blur).b;
	//col = texture2D(renderTex, coords + texel * blur).rgb;
	vec3 lumcoeff = vec3(0.299,0.587,0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum-threshold)*gain, 0.0);
	return col + mix(vec3(0.0),col,thresh*blur);
}

vec2 rand(vec2 coord) //generating noise/pattern texture for dithering
{
	float noiseX = ((fract(1.0-coord.s*(width/2.0))*0.25)+(fract(coord.t*(height/2.0))*0.75))*2.0-1.0;
	float noiseY = ((fract(1.0-coord.s*(width/2.0))*0.75)+(fract(coord.t*(height/2.0))*0.25))*2.0-1.0;

	if (noise)
	{
		noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
		noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
	}
	return vec2(noiseX,noiseY);
}

float linearize(float depth)
{
	return -zfar * znear / (depth * (zfar - znear) - zfar);
}

vec3 computeAverageBlur(float w, float h, float blur)
{
	vec3 col = texture2D(renderTex, texCoord.xy).rgb;
	float s = 1.0;
	int ringsamples;

	for (int i = 1; i <= rings; i += 1)
	{   
		ringsamples = i * samples;

		for (int j = 0 ; j < ringsamples ; j += 1)   
		{
			float step = PI*2.0 / float(ringsamples);
			float pw = (cos(float(j)*step)*float(i));
			float ph = (sin(float(j)*step)*float(i));
			col += color(texCoord.xy + vec2(pw*w,ph*h),blur)*mix(1.0,(float(i))/(float(rings)),bias);  
			s += 1.0*mix(1.0,(float(i))/(float(rings)),bias);   
		}
	}
	col /= s;
	return col;
}

float G(float x, float y, float sigma)
{
	float s = sigma * sigma;	
	return 1.0 / (2.0 * PI * s) * exp(-(x*x+y*y)/(2.0*s));
}

//vec3 computeGaussianBlur()
//{
//	vec3 color = vec3(0);
//	vec2 offset = 1.0 / textureSize(renderTex, 0);
//	float sum = 0.0;
//	float sigma = 3.0;
//	int kernelSize = 5;
//	float k = kernelSize / 2;
//	for(float y = -k; y <= k; y++)
//	{
//		for(float x = -k; x <= k; x++)
//		{
//			float w = G(x,y,sigma);
//			sum += w;
//		}
//	}
//	for(float y = -k; y <= k; y++)
//	{
//		for(float x = -k; x <= k; x++)
//		{
//			vec2 texel = texCoord + vec2(offset.x * x, offset.y * y);
//			float w = G(x,y,sigma) / sum;
//			color += w * texture(renderTex, texel).rgb;
//		}
//	}
//
//	return color;
//}

float G_Fixed(float x, float a, float b, float c)
{	
	return a * exp(-pow((x - b)/c,2.0));
}

vec3 computeGaussianBlur(float a, float b, float c, float w, float h, float blur)
{
	vec3 col = texture2D(renderTex, texCoord.xy).rgb;
	int ringsamples;
	float sum = 0.0f;
	for (int i = 1; i <= rings; i += 1)
	{   
		ringsamples = i * samples;

		for (int j = 0 ; j < ringsamples ; j += 1)   
		{
			float step = PI*2.0 / float(ringsamples);
			float pw = (cos(float(j)*step)*float(i));
			float ph = (sin(float(j)*step)*float(i));
			float px = pw*w;
			float py = ph*w;
			float r = sqrt(px * px + py * py);
			float w = G_Fixed(r, a, b, c);
			sum += w;
		}
	}

	float numSamples = 0.0;
	for (int i = 1; i <= rings; i += 1)
	{   
		ringsamples = i * samples;

		for (int j = 0 ; j < ringsamples ; j += 1)   
		{
			float step = PI*2.0 / float(ringsamples);
			float pw = (cos(float(j)*step)*float(i));
			float ph = (sin(float(j)*step)*float(i));
			
			float px = pw*w;
			float py = ph*w;
			float r = sqrt(px * px + py * py);

			float w = G_Fixed(r, a, b, c) / sum;
			col += color(texCoord.xy + vec2(px,py),blur) * w;
			numSamples++;
		}
	}
	//col /= numSamples;
	return col;
}

vec3 computeGMMBlur(float w, float h, float blur)
{
	vec3 col = texture2D(renderTex, texCoord.xy).rgb;
	float s = 1.0;
	int ringsamples;
	float numSamples = 0.0;

	for (int i = 1; i <= rings; i += 1)
	{   
		ringsamples = i * samples;
		for (int j = 0 ; j < ringsamples ; j += 1)   
		{
			float step = PI*2.0 / float(ringsamples);
			float pw = (cos(float(j)*step)*float(i));
			float ph = (sin(float(j)*step)*float(i));
			
			float px = pw*w;
			float py = ph*w;
			float r = sqrt(px * px + py * py);

			float w1 = G_Fixed(r, a1, b1, c1);
			float w2 = G_Fixed(r, a2, b2, c2);
			float w3 = G_Fixed(r, a3, b3, c3);

			vec3 sampleColor = vec3(0);
			sampleColor += color(texCoord.xy + vec2(px,py),blur)*w1;  
			sampleColor += color(texCoord.xy + vec2(px,py),blur)*w2;  
			sampleColor += color(texCoord.xy + vec2(px,py),blur)*w3;  
			col += sampleColor;
			numSamples++;
		}
	}
	//col /= numSamples;
	return col;
}

void main() 
{
	float zValue = texture2D(depthTex,texCoord.xy).r;
	
	//scene depth calculation
	float depth = linearize(zValue);

	//focal plane calculation
	float fDepth = focalDepth;
	if (autofocus)
	{
		fDepth = linearize(texture2D(depthTex,focus).r);
		//fDepth = linearize(texture2D(renderTex,focus).w);
	}

	//dof blur factor calculation
	float blur = 0.0;
	float f = focalLength; //focal length in mm
	float d = fDepth*1000.0; //focal plane in mm
	float o = depth*1000.0; //depth in mm

	float a = (o*f)/(o-f); 
	float b = (d*f)/(d-f); 
	float c = (d-f)/(d*fstop*CoC);

	blur = abs(a-b)*c;
	//blur = clamp(blur,0.0,1.0);

	// calculation of pattern for ditering
	vec2 noise = rand(texCoord.xy)*namount*blur;

	// getting blur x and y step factor 
	float w = (1.0/width)*blur*maxblur+noise.x;
	float h = (1.0/height)*blur*maxblur+noise.y;

	// calculation of final color
	vec3 col = vec3(0.0);
	if(blur < 0.05) //some optimization thingy
	{
		col = texture2D(renderTex, texCoord.xy).rgb;
	}
	else
	{
		//col = computeAverageBlur(w,h,blur);
		col = computeGMMBlur(w, h, blur);
	}

	//float exposure = 0.12;
	//col *= 0.0375;
	//col *= 1.1;
	col *= 0.125;
	//col = vec3(1.0) - exp(-col * exposure); // EV
	col = pow(col, vec3(1.0 / 2.2));

////	if(texCoord.x < 0.4 || texCoord.x > 0.9 || texCoord.y < 0.4 || texCoord.y > 0.65)
//	if(depth > 100.0)
//	{
//		col = texture2D(renderTex, texCoord.xy).rgb;
//		//col = vec3(1.0) - exp(-col * exposure); // EV 
//		col = pow(col, vec3(1.0 / 2.2));
////		col = vec3(0);
//		//outColor = vec4(1.0);
//	}

	outColor = vec4(col, 1.0);
}
