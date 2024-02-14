
#define PI  3.14159265

uniform float width;
uniform float height;
vec2 texel = vec2(1.0/width,1.0/height);

uniform float focalDepth;
uniform float focalLength = 4.3;
uniform float fstop = 1.5;

uniform float znear = 0.1;
uniform float zfar = 100.0;

//------------------------------------------
//user variables

uniform int samples = 5; //samples on the first ring
uniform int rings = 3; //ring count

uniform float CoC = 0.03;//circle of confusion size in mm (35mm film = 0.03mm)

bool autofocus = false; //use autofocus in shader? disable if you use external focalDepth value
vec2 focus = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)
float maxblur = 2.0; //clamp value of max blur (0.0 = no blur,1.0 default)

uniform float threshold = 1.0; //highlight threshold;
uniform float gain = 100.0; //highlight gain;
uniform vec3 gainTint;

float bias = 0.0; //bokeh edge bias
float fringe = 0.0; //bokeh chromatic aberration/fringing

bool noise = false; //use noise instead of pattern for sample dithering
float namount = 0.0001; //dither amount

uniform float a1, a2, a3;
uniform float b1, b2, b3;
uniform float c1, c2, c3;

float linearize(float depth)
{
	return -zfar * znear / (depth * (zfar - znear) - zfar);
}

vec3 color(vec2 coords,float blur) //processing the sample
{
	vec3 col = vec3(0.0);

//	float zValue = texture2D(depthTex,coords).r;
//	float depth = linearize(zValue);
//	if(depth < 0.12)
//		return col;

	col.r = texture2D(linearRGBTex,coords + vec2(0.0,0.0)*texel*fringe*blur).r;
	col.g = texture2D(linearRGBTex,coords + vec2(0.0,-1.0)*texel*fringe*blur).g;
	col.b = texture2D(linearRGBTex,coords + vec2(0,0)*texel*fringe*blur).b;
	//col = texture2D(renderTex, coords + texel * blur).rgb;
	vec3 lumcoeff = vec3(0.299,0.587,0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum-threshold)*gain, 0.0);
	return col + mix(vec3(0.0),col,thresh*blur) * gainTint;
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

vec3 computeAverageBlur(float w, float h, float blur)
{
	vec3 col = texture2D(linearRGBTex, texCoord0.xy).rgb;
	float s = 1.0;
	int ringsamples;
	for (int i = 1; i <= rings; i += 1)
	{   
		ringsamples = i * samples;

		for (int j = 0 ; j < ringsamples ; j += 1)   
		{
			float step = PI*2.0 / float(ringsamples);
			float pw = cos(float(j)*step)*float(i);
			float ph = sin(float(j)*step)*float(i);

			col += color(texCoord0.xy + vec2(pw*w,ph*h),blur)*mix(1.0,(float(i))/(float(rings)),bias);  
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
	vec3 col = texture2D(linearRGBTex, texCoord0.xy).rgb;
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
			col += color(texCoord0.xy + vec2(px,py),blur) * w;
			numSamples++;
		}
	}
	//col /= numSamples;
	return col;
}

vec3 computeGMMBlur(float w, float h, float blur)
{
	vec3 col = texture2D(linearRGBTex, texCoord0.xy).rgb;
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
			sampleColor += color(texCoord0.xy + vec2(px,py),blur)*w1;  
			sampleColor += color(texCoord0.xy + vec2(px,py),blur)*w2;  
			sampleColor += color(texCoord0.xy + vec2(px,py),blur)*w3;  
			col += sampleColor;
			numSamples++;
		}
	}
	col /= numSamples;
	return col;
}

float avgDepth(vec2 texCoord)
{
	float depth = 0.0;
	vec2 texelSize = 1.0 / vec2(textureSize(depthTex, 0));
	for (int x = -5; x <= 5; x++)
	{
		for (int y = -5; y <= 5; y++)
		{
			float zValue = texture(depthTex, texCoord + vec2(x, y) * texelSize).r;
			depth += linearize(zValue);
		}
	}
	return depth / 121.0;
}

uniform float maxDepth = 0.0;

vec3 computeDoF() 
{
	float zValue = texture2D(depthTex,texCoord0.xy).r;

	//scene depth calculation
	float depth = linearize(zValue);
	//float depth = avgDepth(texCoord0.xy);

	//if(depth > 0.99)
	//	return texture2D(linearRGBTex, texCoord0.xy).rgb;
	
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
	vec2 noise = rand(texCoord0.xy)*namount*blur;

	// getting blur x and y step factor 
	float w = (1.0/width)*blur*maxblur+noise.x;
	float h = (1.0/height)*blur*maxblur+noise.y;
	
	vec3 col = vec3(0.0);
	// calculation of final color
	if(blur < 0.05) //some optimization thingy
	{
		col = texture2D(linearRGBTex, texCoord0.xy).rgb;
	}
	else
	{
		col = computeAverageBlur(w,h,blur);
		//col = computeGMMBlur(w, h, blur);
	}

	return col;
}
