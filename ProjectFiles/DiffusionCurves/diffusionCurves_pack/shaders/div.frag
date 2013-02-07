/*
Copyright 2007 Adobe Systems Incorporated.  All rights reserved.

All parties may modify, copy and distribute this software in source and binary forms, with or without modification, solely for non-commercial research purposes, provided that the following conditions are met:

·        Distributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

·        Distributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

·        Neither the name of Adobe Systems Incorporated nor the names of its licensors or contributors may be used for any endorsement without specific prior written permission.

 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


uniform sampler2DRect gxMap, gyMap;

void main()
{
	vec2 pixCoord = gl_FragCoord.xy;

	//div = dgx/dx + dgy/dy
        vec4 gxG = texture2DRect(gxMap,pixCoord-vec2(1,0));
	vec4 gxD = texture2DRect(gxMap,pixCoord+vec2(1,0));
	vec4 gyH = texture2DRect(gyMap,pixCoord-vec2(0,1));
	vec4 gyB = texture2DRect(gyMap,pixCoord+vec2(0,1));


	gl_FragColor = vec4(0.5,0.5,0.5,1)+0.5*(gxG-gxD+gyH-gyB);
}
