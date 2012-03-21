/* Design Implementation - Application Layer
 *
 * Design.cpp
 * fwk4gps version 3.0
 * gam666/dps901/gam670/dps905
 * January 14 2012
 * copyright (c) 2012 Chris Szalwinski 
 * distributed under TPL - see ../Licenses.txt
 */

#include "Design.h"          // for the Design class definition
#include "iText.h"           // for the Text Interface
#include "iHUD.h"            // for the HUD Interface
#include "iSound.h"          // for the Sound Interface
#include "iLight.h"          // for the Light Interface
#include "iObject.h"         // for the Object Interface
#include "iTexture.h"        // for the Texture Interface
#include "Camera.h"          // for the Camera Interface
#include "iGraphic.h"        // for the Graphic Interface
#include "iUtilities.h"      // for strcpy()
#include "MathDefinitions.h" // for MODEL_Z_AXIS
#include "ModellingLayer.h"  // for MOUSE_BUTTON_SCALE, ROLL_SPEED
#include "Common_Symbols.h"  // for Action and Sound enumerations

const wchar_t* orient(wchar_t*, const iFrame*, char, unsigned = 1u);
const wchar_t* position(wchar_t*, const iFrame*, char = ' ', unsigned = 1u);
const wchar_t* onOff(wchar_t*, const iSwitch*);

//-------------------------------- Design -------------------------------------
//
// The Design class implements the game design within the Modelling Layer
//
// constructor initializes the engine and the instance pointers
//
Design::Design(void* h, int s) : Coordinator(h, s) {

	// pointers to the objects
    rollLeft     = nullptr;
    rollRight    = nullptr;
	spinTop      = nullptr;
    child        = nullptr;

	// pointers to the lights
    pointLight   = nullptr;
	spotLight    = nullptr;
	distantLight = nullptr;
    topLight     = nullptr;
    cloneLight   = nullptr;

    // pointers to the sounds
	background   = nullptr;
	discrete     = nullptr;
	locall       = nullptr;
	localr       = nullptr;
	objectSnd    = nullptr;
    cloneSound   = nullptr;
}

// initialize initializes the general display design coordinator, creates the 
// primitive sets, textures, objects, lights, sounds, cameras, and text items
//
void Design::initialize() {

    // general display design
    //
    setProjection(0.9f, 1.0f, 1000.0f);
    setAmbientLight(0.2f, 0.2f, 0.2f);
    setBackground(CreateTexture(L"whirlpool.jpg"));
    hud = CreateHUD(0.1f, 0.1f, 0.43f, 0.43f, CreateTexture(HUD_IMAGE));
	setTimerText(CreateText(Rectf(0.0f, 0.05f, 0.2f, 0.15f), hud, L"", 
     TEXT_HEIGHT, TEXT_TYPEFACE, TEXT_LEFT));
	CreateText(Rectf(0, 0.05f, 0.65f, 0.15f), hud, L" Camera: at ", position, 
     Camera::getCurrent(), ' ', 1, 16, L"ARIAL", TEXT_CENTER);
    CreateText(Rectf(0.50f, 0.05f, 0.90f, 0.15f), hud, L" heading ", orient, 
     Camera::getCurrent(), 'z', 100,  16, L"ARIAL", TEXT_CENTER);

    // cameras ----------------------------------------------------------------

    // camera at a distance - in lhs coordinates
    camera = CreateCamera();
    camera->translate(-20, 0, -80);
    camera->setRadius(17.8f);

    // camera attached to the left box object
    iCamera* objectCamera = CreateCamera();

    // model ------------------------------------------------------------------

    Colour white(1, 1, 1);
    Colour yellow(0.7f, 0.7f, 0.0f);
    Colour grey(0.7f, 0.7f, 0.7f);
    Colour blue(0, 0.3f, 0.9f);
    Colour turquoise(0, 0.8f, 0.6f);
    Colour black(0, 0, 0);

	// create textures
	iTexture* checkbmp = CreateTexture(L"check.bmp", TEX_MIN_ANISOTROPIC | \
                         TEX_MAG_ANISOTROPIC);
	iTexture* checktga = CreateTexture(L"check.tga");
	iTexture* checkdsy = CreateTexture(L"daisy.bmp", TEX_MIN_ANISOTROPIC | \
                         TEX_MAG_ANISOTROPIC);
    iTexture* clouds   = CreateTexture(L"clouds.png");

    // create vertex lists
    iGraphic* box   = CreateBox(-10, -10, -10 * MODEL_Z_AXIS, 
     10, 10, 10 * MODEL_Z_AXIS);
    iGraphic* plate = CreateBox(-50, -10, -50 * MODEL_Z_AXIS, 
     50, 10, 50 * MODEL_Z_AXIS);
    iGraphic* grid  = CreateGrid(-25, 25, 10);
    iGraphic* square = TriangleList(L"colouredsquare.txt", yellow);
    CreateObject(square);
    iGraphic* cbox  = CreateBox(-5, -5, -5 * MODEL_Z_AXIS, 
     5, 5, 5 * MODEL_Z_AXIS);

    Reflectivity greyish = Reflectivity(grey);
    rollRight = CreateObject(box, &greyish);
    rollRight->setAxisAligned(Vector(-10, -10, -10), Vector(10, 10, 10));
	rollRight->attach(checkdsy);

    Reflectivity blueish = Reflectivity(blue);
    child = CreateObject(cbox, &blueish);
    child->translate(0, 0, 10 * MODEL_Z_AXIS);
    child->attachTo(rollRight);

    rollRight->translate(20, -20, 40 * MODEL_Z_AXIS);
    rollRight->setPlane(Vector(-1,0,0), 10);

    Reflectivity greenish = Reflectivity(Colour(0.1f, 0.8f, 0.1f, 0.5f));
    rollLeft = CreateObject(box, &greenish);
    rollLeft->setAxisAligned(Vector(-10, -10, -10), Vector(10, 10, 10));
	rollLeft->attach(checktga);
    rollLeft->translate(-23, 13, 30 * MODEL_Z_AXIS);
    rollLeft->setRadius(17.8f);
    objectCamera->attachTo(rollLeft);

    Reflectivity bluish = Reflectivity(Colour(0.0f, 0.1f, 0.9f));
    floor = CreateObject(plate, &bluish);
    floor->setAxisAligned(Vector(-50, -10, -50), Vector(50, 10, 50));
	floor->attach(checkdsy);
	floor->translate(-10, -63, 180 * MODEL_Z_AXIS);

    Reflectivity redisher = Reflectivity(Colour(0.9f, 0.1f, 0.1f));
    spinTop = CreateObject(box, &redisher);
	spinTop->attach(checkbmp);
    spinTop->translate(-10, 30, 50 * MODEL_Z_AXIS);
	spinTop->rotatex(1.57f * MODEL_Z_AXIS);

    Reflectivity whiteish = Reflectivity(Colour(0.9f, 0.9f, 0.9f));
    iObject* xz = CreateObject(grid, &whiteish);
    xz->setRadius(71);
    iObject* xy = Clone(xz);
    iObject* yz = Clone(xz);
    xz->translate(25, 0, 25 * Z_AXIS);
    xy->rotatex(1.5708f);
    xy->translate(25, 25, 0);
    yz->rotatez(1.5708f);
    yz->translate(0, 25, 25 * Z_AXIS);

    // lighting ---------------------------------------------------------------

    // Create far away point-source light
	pointLight = CreatePointLight(grey, grey, white, 20000.0f, true);
    pointLight->translate(500.f, 1000.f, 100.f * MODEL_Z_AXIS);

    // create overhead spotlight
	spotLight = CreateSpotLight(white, white, white, 300.0f, true, 1, 0.00005f, 
	 0.00001f, .60f, .25f, 0.9f);
    spotLight->translate(-20.f, 10.f, -50.f * MODEL_Z_AXIS);
    spotLight->rotatex(.7f);

    // create directional light
 	distantLight = CreateDistantLight(turquoise, turquoise, black, false);
    distantLight->translate(10, 1000, 0);
    distantLight->rotatex(1.57f);
    distantLight->rotatez(0.20f);

     // create a spotlight attached to the top object
    topLight = CreateSpotLight(white, white, white, 200.0f, true, 1, 0.0005f, 
     0, .30f, .25f, 0.1f);
    topLight->rotatex(.7f);
    topLight->attachTo(spinTop);

	// audio ------------------------------------------------------------------
    
    // create background sound
	if (soundFile(SND_BKGRD))
		background = CreateSound(soundFile(SND_BKGRD), false, true, false);

	// create discrete sound
	if (soundFile(SND_DISCR))
		discrete = CreateSound(soundFile(SND_DISCR), false, false, false);

	// create local sound on the left
	if (soundFile(SND_LOCAL_L)) {
		locall = CreateSound(soundFile(SND_LOCAL_L), true, true, true, 90);
		locall->translate(-30, 5, 40 * MODEL_Z_AXIS);
		locall->rotatey(3.14f);
	}

	// create local sound on the right
	if (soundFile(SND_LOCAL_R)) {
		localr = CreateSound(soundFile(SND_LOCAL_R), true, true, true, 90);
		localr->translate(30, 5, 40 * MODEL_Z_AXIS);
		localr->rotatey(3.14f);
	}

	// create a local sound attached to right object
	if (soundFile(SND_OBJECT)) {
		objectSnd = CreateSound(soundFile(SND_OBJECT), true, true, true, 90);
        objectSnd->attachTo(rollRight);
    }

    // Heads Up Display Text --------------------------------------------------

    // object data
    CreateText(Rectf(0, 0.30f, 0.7f, 0.42f), hud, L" Spinner x orientation ",
     orient, rollLeft, 'x', 100);
    CreateText(Rectf(0, 0.42f, 0.7f, 0.54f), hud, L" Spinner y orientation ", 
     orient, rollLeft, 'y', 100);
    CreateText(Rectf(0, 0.54f, 0.7f, 0.66f), hud, L" Spinner z orientation ", 
     orient, rollLeft, 'z', 100);

    // lighting data
	CreateText(Rectf(0, 0.66f, 0.5f, 0.74f), hud, L"Point light ", onOff, 
     pointLight);
	CreateText(Rectf(0, 0.74f, 0.5f, 0.82f), hud, L"Spot light ", onOff,
     spotLight);
 	CreateText(Rectf(0, 0.82f, 0.5f, 0.90f), hud, L"Directional light ",
     onOff, distantLight);
	CreateText(Rectf(0, 0.90f, 0.5f, 0.98f), hud, L"Top light ", onOff,
     topLight);

    // sound data
	CreateText(Rectf(0.5f, 0.66f, 1, 0.74f), hud, L"Background ", onOff,
     background);
	CreateText(Rectf(0.5f, 0.74f, 1, 0.82f), hud, L"Discrete ", onOff, 
     discrete);
	CreateText(Rectf(0.5f, 0.82f, 1, 0.90f), hud, L"Local left ", onOff,
     locall);
	CreateText(Rectf(0.5f, 0.90f, 1, 0.98f), hud, L"Local right ", onOff,
     localr);
}

// update updates the position and orientation of each object according to the 
// actions initiated by the user
//
void Design::update() {

    int delta = now - lastUpdate;
    int dr = 0;  // roll the right box around its x axis
    int ds = 0;  // spin the right box around its y axis
    int dt = 0;  // roll the top   box around its z axis
    int dw = 0;  // roll the spot light around the x axis
    int tx = 0;
    int ty = 0;
    int tz = 0;

	// audio ------------------------------------------------------------------
    
    if (pressed(AUD_BKGRD))
        background->toggle();
    if (pressed(AUD_IMPLS) || ctrPressed())
        discrete->toggle();

    // lighting ---------------------------------------------------------------

    if (pressed(LIGHT_POINT) && pointLight)
		pointLight->toggle();
	if (pressed(LIGHT_SPOT) && spotLight)
		spotLight->toggle();
	if (pressed(LIGHT_DISTANT) && distantLight)
		distantLight->toggle();
	if (pressed(LIGHT_CLONE) && cloneLight)
		cloneLight->toggle();

    // model ------------------------------------------------------------------

    // add changes introduced by user input
    if (pressed(MDL_ROLL_BOXES))
        dr += delta;
    if (pressed(MDL_SPIN_LEFT))
        ds += delta;
    if (pressed(MDL_SPIN_TOP))
        dt += delta;
    if (pressed(MDL_SPIN_SPOT_P))
        dw += delta;
    if (pressed(MDL_SPIN_SPOT_N))
        dw -= delta;

    // translation of rollright box
    if (pressed(MDL_PLUS_X))
        tx += delta;
    if (pressed(MDL_MINUS_X))
        tx -= delta;
    if (pressed(MDL_PLUS_Y))
        ty += delta;
    if (pressed(MDL_MINUS_Y))
        ty -= delta;
    if (pressed(MDL_PLUS_Z))
        tz += delta;
    if (pressed(MDL_MINUS_Z))
        tz -= delta;

    // translation and possible collision
    Vector d((float)tx, (float)ty, (float)tz);
    d *= FORWARD_SPEED;
    if (rollRight) 
	    rollRight->translate(d.x, d.y, d.z);
    if (collision(floor, rollRight, d))
        rollRight->translate(d.x, d.y, d.z);

	// adjust the boxes' positions and orientations for user input
    if (rollRight) 
	     rollRight->rotatex(dr * ROT_SPEED + CONSTANT_ROLL);
    if (rollLeft) 
	    rollLeft->rotatex(dr * ROT_SPEED + CONSTANT_ROLL);
    if (dt && spinTop) 
		spinTop->rotatez(dt * ROT_SPEED * MODEL_Z_AXIS);
    if (ds && rollLeft)
        rollLeft->rotatey(ds * ROT_SPEED);
    if (dw && spotLight)
        spotLight->rotatex(dw * ROT_SPEED);
    
    // clone object, sound, light - restrict to one clone each 
    static bool co = true;
    static bool cs = true;
    static bool cl = true;
    if (pressed(MDL_NEW_OBJECT) && co) {
        co = false;
        Clone(rollRight)->translate(-40, -20, 30 * MODEL_Z_AXIS);
    }
    if (pressed(MDL_NEW_LIGHT) && cl) {
        cl = false;
        (cloneLight = Clone(topLight))->attachTo(rollLeft);
     	CreateText(Rectf(0.50f, 0.54f, 0.90f, 0.66f), hud, 
         L"Cloned light ", onOff, cloneLight);
    }
    if (pressed(MDL_NEW_SOUND) && cs) {
        cs = false;
        (cloneSound = Clone(objectSnd))->translate(-60, 0, 0);
    }

    // attach child object
    if (pressed(MDL_ATT_CHILD) && child && rollRight)
        child->attachTo(rollRight);
    if (pressed(MDL_DET_CHILD) && child && rollRight)
        child->attachTo(nullptr);

    // respond to collision with left object
    if (collision(camera, rollLeft, d) && spotLight) {
        if (!spotLight->isOn()) spotLight->toggle();
    }
    else 
        if (spotLight && spotLight->isOn()) spotLight->toggle();

    // respond to collision with right object
    if (collision(camera, rollRight, d) && distantLight) {
        if (!distantLight->isOn()) distantLight->toggle();
    }
    else 
        if (distantLight && distantLight->isOn()) distantLight->toggle();
}
