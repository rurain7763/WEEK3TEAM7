cbuffer Constants : register(b0) // FConstants
{
	float4 color;
    float3 start;
    float thickness;
    float3 end;
	float3 padding;
}

cbuffer CameraConstants : register(b1) // CameraConstants
{
    row_major float4x4 view_projection;
}

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

float2 Perpendicular(float2 v)
{
    return float2(-v.y, v.x);
}

// Vertex Shader
PS_INPUT mainVS(uint vertex_id : SV_VertexID)
{
	PS_INPUT output;
	
    float4 proj_start = mul(float4(start, 1.f), view_projection);
    float3 proj_start_ndc = proj_start.xyz / proj_start.w;
    
    float4 proj_end = mul(float4(end, 1.f), view_projection);
    float3 proj_end_ndc = proj_end.xyz / proj_end.w;
    
    float3 normal = normalize(float3(Perpendicular(proj_end_ndc.xy - proj_start_ndc.xy), 0.f));
    float2 offset = normal.xy * thickness * 0.5f;
    
    float4 start_top = proj_start;
    float4 start_bottom = proj_start;
    float4 end_top = proj_end;
    float4 end_bottom = proj_end;
    
    start_top.xy += offset * proj_start.w;
    start_bottom.xy -= offset * proj_start.w;
    end_top.xy += offset * proj_end.w;
    end_bottom.xy -= offset * proj_end.w;
    
    float4 positions[4] =
    {
        start_top,
        end_top,
        end_bottom,
        start_bottom
    };
    
    int indices[6] = { 0, 1, 2, 0, 2, 3 };
    
    output.position = positions[indices[vertex_id]];
    output.color = color;
	
	return output;
}

// Pixel Shader
float4 mainPS(PS_INPUT input) : SV_TARGET
{
	return input.color;
}
