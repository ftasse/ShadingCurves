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


uniform sampler2DRect imgMap;

void main()
{
	vec2 pixCoord = gl_FragCoord.xy;

        int i,j;
        vec4 sum = vec4(0,0,0,0);
        vec4 img;
        int nbFrags = 0;
        for(i=-1;i<=1;i++){
            for(j=-1;j<=1;j++){
                img = texture2DRect(imgMap,2.0*pixCoord+vec2(i,j));
                if(img[3]>0.1){
                    sum+=img;
                    nbFrags+=1;
                }
            }
        }

        if(nbFrags>0){
            sum = sum/float(nbFrags);
            sum[3] = 1.0;
        }
        else{
            sum=vec4(0.0,0.0,0.0,0.0);
        }

	gl_FragColor = sum;
}
