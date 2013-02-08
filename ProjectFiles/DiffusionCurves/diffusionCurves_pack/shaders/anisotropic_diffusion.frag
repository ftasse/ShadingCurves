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


uniform sampler2DRect imgMap, blurMap;
uniform float blur;

void main()
{
	vec2 pixCoord = gl_FragCoord.xy;

	//pixel and neighbours
        vec4 imgH = texture2DRect(imgMap,pixCoord+vec2(0,-1));
        vec4 imgG = texture2DRect(imgMap,pixCoord+vec2(-1,0));
        vec4 img = texture2DRect(imgMap,pixCoord);
	vec4 imgD = texture2DRect(imgMap,pixCoord+vec2(1,0));
        vec4 imgB = texture2DRect(imgMap,pixCoord+vec2(0,1));

        //blur values of the neighbours
        vec4 blurH = texture2DRect(blurMap,pixCoord+vec2(0,-1));
        vec4 blurG = texture2DRect(blurMap,pixCoord+vec2(-1,0));
	vec4 blurD = texture2DRect(blurMap,pixCoord+vec2(1,0));
	vec4 blurB = texture2DRect(blurMap,pixCoord+vec2(0,1));

        if(blurG[0]>=blur){
            blurG[0]=0.25;
        }
        else{
            blurG[0]=0.0;
        }

        if(blurD[0]>=blur){
            blurD[0]=0.25;
        }
        else{
            blurD[0]=0.0;
        }

        if(blurH[0]>=blur){
            blurH[0]=0.25;
        }
        else{
            blurH[0]=0.0;
        }

        if(blurB[0]>=blur){
            blurB[0]=0.25;
        }
        else{
            blurB[0]=0.0;
        }

        

        float w = 1.0-blurG[0]-blurD[0]-blurH[0]-blurB[0];

	vec4 result = w*img+blurG[0]*imgG+blurD[0]*imgD+blurH[0]*imgH+blurB[0]*imgB;

        result = clamp(result,vec4(0,0,0,1),vec4(1,1,1,1));

        gl_FragColor = result;
}
