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
float2 gTopLeft;
float2 gBottomRight;

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
	float3 sampled = gTexture.Sample(samPoint, input.TexCoord);
	
	float width = 0;
	float height = 0;
	gTexture.GetDimensions(width, height);
	
	float x = input.Position.x;//(input.Position.x / width + 1) / 2.f;
	float y = input.Position.y;//1-(input.Position.y / height) / 2.f;
	bool isInSquare = x >= gTopLeft.x && x <= gBottomRight.x
		&& y >= gBottomRight.y && y <= gTopLeft.y;
	
	float additional = 0.3f;
	
	float3 color = isInSquare ? saturate(float3(sampled.x + additional, 
		sampled.y + additional, sampled.z + additional)) : sampled;
	
    return float4(color, 1.0f);
}


//TECHNIQUE
//---------
technique11 Highlight
{
    pass P0
    {          
        // Set states...
		SetRasterizerState(rasterizerState);
		SetDepthStencilState(depthStencilState, 0);
		
		SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

