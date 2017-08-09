#ifndef TOOLS_H
#define TOOLS_H

void genvertices(GLfloat *vert, int nx, int ny){
    float pos_x = -0.5;
    float pos_y = 0.0;
    float dx = 0.1;
    float dy = 0.1;

    for (int y = 0; y < ny; ++y)          //CANTIDAD DE LINEAS DE TRIANGULOS
    {
        for (int x = 0; x < nx; ++x)            //cant de puntos
        {
        	vert[(y*nx + x)*3] = pos_x + dx*x;
        	vert[(y*nx + x)*3 + 1] =  pos_y + dy*y;
            vert[(y*nx + x)*3 + 2] =  0.0f;
        }
    }
}


void genindices(GLuint *indices, int nx, int ny){

    int cont = 0;
    for (int y = 0; y < ny-1; ++y)          //CANTIDAD DE LINEAS DE TRIANGULOS
    {
        for (int x = 0; x < nx-1; ++x)            //cant de puntos
        {
        	indices[cont*3 + 0] = y*nx + x;
        	indices[cont*3 + 1] = (y+1)*nx + x;
        	indices[cont*3 + 2] = y*nx + (x+1);
            cont++;
        }
    }
}

void gencolors(GLfloat *colors, int n){
    for (int i = 0; i < n; ++i)          //CANTIDAD DE LINEAS DE TRIANGULOS
    {
        colors[i*3 + 0] = 0.5f;
        colors[i*3 + 1] = 0.5f;
        colors[i*3 + 2] = 0.5f;
    }
}

#endif