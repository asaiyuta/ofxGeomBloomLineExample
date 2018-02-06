#include "ofMain.h"


class ofApp : public ofBaseApp{
    static constexpr std::size_t numLines = 36;
    static constexpr float rad = 100;

public:
   
    void setup(){
        verts.resize(numLines);
        colors.assign(numLines, ofFloatColor(1,0,0,1));
        float d = numLines / 2.0;
        for(std::size_t i = 0 ; i < numLines / 2 ; ++i){
            float rad_0 =  TWO_PI * (i) / d;
            float rad_1 =  TWO_PI * (i + 1) / d;
            verts[i * 2].set(0, cos(rad_0) * rad, sin(rad_0) * rad);
            verts[i * 2 + 1].set(0, cos(rad_1) * rad, sin(rad_1) * rad);
        }
        vbo.setVertexData(verts.data(), numLines, GL_DYNAMIC_DRAW);
        vbo.setColorData(colors.data(), numLines, GL_DYNAMIC_DRAW);

        initShader();
    }

    void update(){
        if(isMotion){
            float time = ofGetElapsedTimef();
            camPos.set(cos(time) * 300, 0, sin(time) * 300);
            rendercam.setPosition(camPos);
            rendercam.lookAt((-camPos).normalize(), ofVec3f(0,1,0));
            
            if(ofGetElapsedTimeMillis() > currentTime){
                currentTime = ofGetElapsedTimeMillis() + 1000;
                currentColor = ofFloatColor(ofRandom(1.0), ofRandom(1.0), ofRandom(1.0), 1.0);
                colors.assign(numLines, currentColor);
                vbo.updateColorData(colors.data(), numLines);
                
            }
            
            float _rad = 20 * (cos(time * 2.0) + 1.0);
            float __rad = rad + _rad;
            float d = numLines / 2.0;
            for(std::size_t i = 0 ; i < numLines / 2 ; ++i){
                float rad_0 =  TWO_PI * (i) / d;
                float rad_1 =  TWO_PI * (i + 1) / d;
                verts[i * 2].set(_rad, cos(rad_0) * __rad, sin(rad_0) * __rad);
                verts[i * 2 + 1].set(-_rad, cos(rad_1) * __rad, sin(rad_1) * __rad);
            }
            
            vbo.updateVertexData(verts.data(), numLines);
        }
    }
    
    void draw(){
        ofBackgroundGradient(ofColor(50), ofColor(0));
        if(isUseRenderCam) rendercam.begin();
        else cam.begin();
        beginShader();
        renderScene();
        endShader();
        
        
        if(isUseRenderCam) rendercam.end();
        else {
            ofMatrix4x4 camMat;
            camMat.setTranslation(rendercam.getPosition());
            ofMultViewMatrix(camMat);
            ofDrawSphere(0,0,0,3);
            cam.end();
        }
        
        std::stringstream ss;
        ss << "camera change = SPACE KEY \n"
        << "moving stop or start = m KEY \n"
        << "line width change = up or down KEY \n";
        ofDrawBitmapString(ss.str(), 20, 20);
    
    }
    
    void renderScene(){
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        vbo.draw(GL_LINES, 0, numLines);
    }
    
    
    void beginShader(){
        ofMatrix4x4 modelMatrix;
        modelMatrix.translate(0, 0, 0);
        
        ofMatrix4x4 viewMatrix;
        viewMatrix = ofGetCurrentViewMatrix();
        
        ofMatrix4x4 projectionMatrix;
        projectionMatrix = cam.getProjectionMatrix();
        
        ofMatrix4x4 mvpMatrix;
        mvpMatrix = modelMatrix * viewMatrix * projectionMatrix;
        
        shader.begin();
        shader.setUniformMatrix4f("model", modelMatrix);
        shader.setUniformMatrix4f("projection", projectionMatrix);
        shader.setUniformMatrix4f("mvp", mvpMatrix);
        shader.setUniform3f("camPosition", rendercam.getPosition());
        shader.setUniform1f("scale", bloomWidth);
    }
    
    void endShader(){
        shader.end();
    }


    void keyPressed(int key){
        if(key == OF_KEY_UP) bloomWidth += 1.0;
        if(key == OF_KEY_DOWN) bloomWidth -= 1.0;
    }
    void keyReleased(int key){
        if(key == 'm') isMotion = !isMotion;
        if(key == ' ') isUseRenderCam = !isUseRenderCam;
    }
    

    
#define _STR_(x) #x
#define STRINGFIY(x) _STR_(x)
    void initShader(){
        std::string vert = "#version 330\n";
        std::string geom = "#version 330\n";
        std::string frag = "#version 330\n";
        
        vert += STRINGFIY(
                          in vec4 color;
                          in vec3 position;
                          
                          out vec4 vColor;
                          out vec3 vPosition;
                          
                          void main() {
                              vColor = color;
                              vPosition = position;
                          }
                          );
        
        
        geom += STRINGFIY(
                          layout (lines) in;
                          layout (triangle_strip, max_vertices = 108) out;
                          
                          uniform mat4 model;
                          uniform mat4 projection;
                          uniform mat4 mvp;
                          uniform vec3 camPosition;
                          uniform float scale;
                          
                          in vec4 vColor[];
                          in vec3 vPosition[];
                          
                          out vec4 gsColor;
                          
                          vec3 rotate(vec3 p, float angle, vec3 axis){
                              vec3 a = normalize(axis);
                              float s = sin(angle);
                              float c = cos(angle);
                              float r = 1.0 - c;
                              mat3 m = mat3(
                                            a.x * a.x * r + c,
                                            a.y * a.x * r + a.z * s,
                                            a.z * a.x * r - a.y * s,
                                            a.x * a.y * r - a.z * s,
                                            a.y * a.y * r + c,
                                            a.z * a.y * r + a.x * s,
                                            a.x * a.z * r + a.y * s,
                                            a.y * a.z * r - a.x * s,
                                            a.z * a.z * r + c
                                            );
                              return m * p;
                          }
                          
                          
                          void main(){
                              vec3 norm = normalize(camPosition - vPosition[0]);
                              vec3 lineDir_0 = normalize(vPosition[1] - vPosition[0]);
                              vec3 lineWidth_0 = normalize(cross(norm, lineDir_0));
                              
                              vec4 color_0 = vColor[0];
                              color_0.a = 0.0;
                              
                              vec4 color_1 = vColor[1];
                              color_1.a = 0.0;
                              
                              float width = scale * (vColor[0].r + vColor[0].g + vColor[0].b) / 3.0;
                              
                              //4triangle out
                              
                              gl_Position = mvp * vec4(vPosition[0], 1.0);
                              gsColor = vColor[0];
                              EmitVertex();
                              
                              gl_Position = mvp * vec4(vPosition[0] + lineWidth_0 * width, 1.0);
                              gsColor = color_0;
                              EmitVertex();
                              
                              gl_Position = mvp * vec4(vPosition[1], 1.0);
                              gsColor = vColor[1];
                              EmitVertex();
                              EndPrimitive();
                              
                              
                              gl_Position = mvp * vec4(vPosition[0] + lineWidth_0 * width, 1.0);
                              gsColor = color_0;
                              EmitVertex();
                              
                              gl_Position = mvp * vec4(vPosition[1] + lineWidth_0 * width, 1.0);
                              gsColor = color_1;
                              EmitVertex();
                              
                              gl_Position = mvp * vec4(vPosition[1], 1.0);
                              gsColor = vColor[1];
                              EmitVertex();
                              EndPrimitive();
                              
                              gl_Position = mvp * vec4(vPosition[0], 1.0);
                              gsColor = vColor[0];
                              EmitVertex();
                              
                              gl_Position = mvp * vec4(vPosition[0] - lineWidth_0 * width, 1.0);
                              gsColor = color_0;
                              EmitVertex();
                              
                              gl_Position = mvp * vec4(vPosition[1], 1.0);
                              gsColor = vColor[1];
                              EmitVertex();
                              EndPrimitive();
                              
                              
                              gl_Position = mvp * vec4(vPosition[0] - lineWidth_0 * width, 1.0);
                              gsColor = color_0;
                              EmitVertex();
                              
                              gl_Position = mvp * vec4(vPosition[1] - lineWidth_0 * width, 1.0);
                              gsColor = color_1;
                              EmitVertex();
                              
                              gl_Position = mvp * vec4(vPosition[1], 1.0);
                              gsColor = vColor[1];
                              EmitVertex();
                              EndPrimitive();
                              
                              float stride = 3.14159265 / 8.0;
                              for(int i = 0 ; i < 8 ; ++i){
                                  float rad_0 = float(i) * stride;
                                  float rad_1 = float(i + 1) * stride;
                                  gl_Position = mvp *  vec4(vPosition[0] + (rotate(lineWidth_0, rad_0, norm) * width), 1.0);
                                  gsColor = color_0;
                                  EmitVertex();
                                  
                                  gl_Position = mvp *  vec4(vPosition[0] + (rotate(lineWidth_0, rad_1, norm) * width), 1.0);
                                  gsColor = color_0;
                                  EmitVertex();
                                  
                                  vec4 col_01 = vColor[0];
                                  col_01.a / 2.0;
                                  gl_Position = mvp * vec4(vPosition[0], 1.0);
                                  gsColor = col_01;
                                  EmitVertex();
                                  EndPrimitive();
                                  
                                  
                                  gl_Position = mvp *  vec4(vPosition[1] + (rotate(-lineWidth_0, rad_0, norm) * width), 1.0);
                                  gsColor = color_1;
                                  EmitVertex();
                                  
                                  gl_Position = mvp *  vec4(vPosition[1] + (rotate(-lineWidth_0, rad_1, norm) * width), 1.0);
                                  gsColor = color_1;
                                  EmitVertex();
                                  
                                  vec4 col_11 = vColor[1];
                                  col_11.a / 2.0;
                                  gl_Position = mvp * vec4(vPosition[1], 1.0);
                                  gsColor = col_11;
                                  EmitVertex();
                                  EndPrimitive();
                                  
                              }
                              
                          }
                          );
        
        frag += STRINGFIY(
                          precision mediump float;
                          
                          in vec4 gsColor;
                          
                          out vec4 outputColor;
                          
                          void main(){
                              float a = pow(gsColor.a,2.0);
                              outputColor = vec4(gsColor.rgb, a);
                          }
                          );
        
        
        shader.setGeometryInputType(GL_LINES);
        shader.setGeometryOutputType(GL_TRIANGLE_STRIP);
        shader.setGeometryOutputCount(108);
        
        
        shader.setupShaderFromSource(GL_VERTEX_SHADER, vert);
        shader.setupShaderFromSource(GL_GEOMETRY_SHADER, geom);
        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, frag);
        shader.bindDefaults();
        shader.linkProgram();
    }
#undef STRINGFIY
#undef _STR_
    
    ofShader shader;
    std::vector<ofVec3f> verts;
    std::vector<ofFloatColor> colors;
    ofVbo vbo;
    ofFloatColor currentColor;
    uint64_t currentTime{0};
    ofVec3f camPos;
    float bloomWidth{5.0};
    ofCamera rendercam;
    ofEasyCam cam;
    bool isMotion{true};
    bool isUseRenderCam{true};
};

int main( ){
    ofGLFWWindowSettings m_s;
    m_s.setGLVersion(3, 3);
    
    m_s.width = 1024;
    m_s.height = 768;
    auto mainWindow = ofCreateWindow(m_s);
    auto mainApp = std::make_shared<ofApp>();
    
    ofRunApp(mainWindow, mainApp);
    ofRunMainLoop();

}

