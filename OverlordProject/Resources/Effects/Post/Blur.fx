//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};

Texture2D gTexture;

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
DepthStencilState depthStencilState
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

/// Create Rasterizer State (Backface culling) 
RasterizerState rasterizerState
{
	FillMode = SOLID;
	CullMode = BACK;	
};

//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
    float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	// Set the Position
	output.Position = float4(input.Position, 1.f);
	// Set the TexCoord
	output.TexCoord = input.TexCoord;
	
	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input): SV_Target
{
	// Step 1: find the dimensions of the texture (the texture has a method for that)	
	float width = 0;
	float height = 0;
	gTexture.GetDimensions(width, height);
	// Step 2: calculate dx and dy (UV space for 1 pixel)	
	float du = 1.f/width;
	float dv = 1.f/height;
	// Step 3: Create a double for loop (5 iterations each)
	float3 color = (float3)0;
	for(int i = 0; i < 5; i++)
	{
		for(int j=0; j<5;j++)
		{	
			//Inside the loop, calculate the offset in each direction. Make sure not to take every pixel but move by 2 pixels each time
			int offsetU = i * 2;
			int offSetV = j * 2;
			//Do a texture lookup using your previously calculated uv coordinates + the offset, and add to the final color
			color += gTexture.Sample(samPoint, float2(input.TexCoord.x + du * offsetU, input.TexCoord.y + dv * offSetV));
		}
	}
	// Step 4: Divide the final color by the number of passes (in this case 5*5)
	color /= 5 * 5;
	// Step 5: return the final color

	return float4(color, 1.0f);
}


//TECHNIQUE
//---------
technique11 Blur
{
    pass P0
    {
		// Set states...
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}