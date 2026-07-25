/*
TODO:

initialize glwf and window context

compile shaders from source.
name a material to use certain shaders

manage the global camera

provide a way to render the frames out to images/gifs etc.


centeral render function
 * for every object in the scene
 * if its static
  * render all its triangles
 * if its an RB
  * update its model uniform
 * if its a deformable
  * update its vertex data


*/

/*
meshes should manage their gpu buffers and reupload when needed
*/