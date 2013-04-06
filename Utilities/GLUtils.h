#ifndef GLUTILS_H
#define GLUTILS_H

//Include OS specific headers

#if defined(_WIN32)
#define WIN32_WINNT 0x0500
#define WINVER 0x0500
#include <windows.h>
#include <psapi.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#endif

//Include opengl headers

#include "Views/glew/GL/glew.h"
#include <GL/glu.h>

bool isEqual(Point3d p, Point3d q)
{
    return (fabs(p.x()-q.x()) < 1e-05 && fabs(p.y()-q.y()) < 1e-05); // && fabs(p.z()-q.z()) < 1e-05
}

GLenum BGRColourFormat()
{
    GLenum inputColourFormat;
#ifdef GL_BGR
    inputColourFormat = GL_BGR;
#else
#ifdef GL_BGR_EXT
    inputColourFormat = GL_BGR_EXT;
#else
#define GL_BGR 0x80E0
    inputColourFormat = GL_BGR;
#endif
#endif
    return inputColourFormat;
}

// colourFormat = GL_RGB32F does not work (on Z930) for high resolutions
bool setupFrameBuffer(GLuint &framebuffer, GLuint &renderbuffer, GLuint &depthbuffer, int width,  int height, GLuint colourFormat=GL_RGBA8,  bool colour=true, bool depth=false)
{
    GLenum status;
    glGenFramebuffersEXT(1, &framebuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);

    //render buffer
    if (colour)
    {
        glGenRenderbuffersEXT(1, &renderbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, colourFormat, width, height);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                     GL_RENDERBUFFER_EXT, renderbuffer);
    }
    //depth buffer
    if (depth)
    {
        glGenRenderbuffersEXT(1, &depthbuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                     GL_RENDERBUFFER_EXT, depthbuffer);
    }
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        qDebug("Could not draw offscreen");
        return false;
    }

    glViewport(0, 0, width, height);
    glRenderMode(GL_RENDER);
    return true;
}

void cleanupFrameBuffer(GLuint &framebuffer, GLuint &renderbuffer, GLuint &depthbuffer)
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    if (renderbuffer>0) glDeleteRenderbuffersEXT(1, &renderbuffer);
    if (depthbuffer>0) glDeleteRenderbuffersEXT(1, &depthbuffer);
    glDeleteFramebuffersEXT(1, &framebuffer);
}

/**
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 * http://nadeausoftware.com/articles/2012/07/c_c_tip_how_get_process_resident_set_size_physical_memory_use
 */
void getCurrentRSS(size_t &phys, size_t &virt)
{
#if defined(_WIN32)
    /* Windows -------------------------------------------------- */
    PROCESS_MEMORY_COUNTERS info;
    GetProcessMemoryInfo( GetCurrentProcess( ), &info, sizeof(info) );
    phys = (size_t)info.WorkingSetSize;
    virt = (size_t)info.PagefileUsage;
    return;

#elif defined(__APPLE__) && defined(__MACH__)
    /* OSX ------------------------------------------------------ */
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if ( task_info( mach_task_self( ), MACH_TASK_BASIC_INFO,
        (task_info_t)&info, &infoCount ) != KERN_SUCCESS )
         phys = (size_t)0L;		/* Can't access? */
    else phys = (size_t)info.resident_size;
    return;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    /* Linux ---------------------------------------------------- */
    long rss = 0L;
    FILE* fp = NULL;
    if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
        return (size_t)0L;		/* Can't open? */
    if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
    {
        fclose( fp );
        return (size_t)0L;		/* Can't read? */
    }
    fclose( fp );
    phys = (size_t)rss * (size_t)sysconf( _SC_PAGESIZE);
    return;

#else
    /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
    phys = (size_t)0L;			/* Unsupported. */
    return
#endif
}

#endif // GLUTILS_H
