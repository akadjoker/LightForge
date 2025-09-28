// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#include "pch.h"
#include "CCameraSceneNode.h"
#include "ISceneManager.h"
#include "IVideoDriver.h"
#include "Keycodes.h"
#include "ICursorControl.h"
#include "irrOS.h"
#include "ICameraSceneNode.h"

namespace irr
{
	namespace scene
	{

		irr::scene::FpsComponent::FpsComponent()
		{

			for (u32 i = 0; i < EKA_COUNT; ++i)
				CursorKeys[i] = false;

			os::Printer::log("Create FPS Camera Component", ELL_INFORMATION);

			// KeyMap.push_back(SKeyMap(EKA_MOVE_FORWARD, irr::KEY_UP));
			// KeyMap.push_back(SKeyMap(EKA_MOVE_BACKWARD, irr::KEY_DOWN));
			// KeyMap.push_back(SKeyMap(EKA_STRAFE_LEFT, irr::KEY_LEFT));
			// KeyMap.push_back(SKeyMap(EKA_STRAFE_RIGHT, irr::KEY_RIGHT));
			// KeyMap.push_back(SKeyMap(EKA_JUMP_UP, irr::KEY_KEY_J));

			KeyMap.push_back(SKeyMap(EKA_MOVE_FORWARD, KEY_KEY_W));
			KeyMap.push_back(SKeyMap(EKA_MOVE_BACKWARD, KEY_KEY_S));
			KeyMap.push_back(SKeyMap(EKA_STRAFE_LEFT, KEY_KEY_A));
			KeyMap.push_back(SKeyMap(EKA_STRAFE_RIGHT, KEY_KEY_D));

			MaxVerticalAngle = 89.0f;

			MoveSpeed = 10.5f;
			RotateSpeed = 1000.0f;
			JumpSpeed = 0.f;

			MouseYDirection = 1.0f;

			LastAnimationTime = 0;

			firstUpdate = true;
			firstInput = true;
			NoVerticalMovement = false;

			FirstMouseInput = false;
			MouseYDirection = 1.0f;
			MaxVerticalAngle = 89.0f;
			MouseSensitivity = 100.0f;
			CameraYaw = 0.0f;
			CameraPitch = 0.0f;
		}

		void FpsComponent::OnReady()
		{

			os::Printer::log("Ready FPS Camera Component", ELL_INFORMATION);
			CursorControl = owner->getSceneManager()->getCursorControl();
			CursorControl->grab();

			ICameraSceneNode *camera = static_cast<ICameraSceneNode *>(owner);

			CameraPosition = camera->getPosition();
		}

		void FpsComponent::allKeysUp()
		{
			for (u32 i = 0; i < EKA_COUNT; ++i)
				CursorKeys[i] = false;
		}

		//! Sets the rotation speed
		void FpsComponent::setRotateSpeed(f32 speed)
		{
			RotateSpeed = speed;
		}

		//! Sets the movement speed
		void FpsComponent::setMoveSpeed(f32 speed)
		{
			MoveSpeed = speed;
		}

		//! Gets the rotation speed
		f32 FpsComponent::getRotateSpeed() const
		{
			return RotateSpeed;
		}

		// Gets the movement speed
		f32 FpsComponent::getMoveSpeed() const
		{
			return MoveSpeed;
		}

		core::vector3df FpsComponent::GetCameraDirection()
		{

			f32 yawRad = CameraYaw * core::DEGTORAD;
			f32 pitchRad = CameraPitch * core::DEGTORAD;

			core::vector3df direction;
			direction.X = cos(pitchRad) * sin(yawRad);
			direction.Y = -sin(pitchRad);
			direction.Z = cos(pitchRad) * cos(yawRad);

			direction.normalize();
			return direction;
		}

		core::vector3df FpsComponent::GetCameraRight()
		{

			f32 yawRad = (CameraYaw + 90.0f) * core::DEGTORAD;

			core::vector3df right;
			right.X = sin(yawRad);
			right.Y = 0.0f;
			right.Z = cos(yawRad);

			right.normalize();
			return right;
		}

		//! Sets the keyboard mapping for this animator
		void FpsComponent::setKeyMap(SKeyMap *map, u32 count)
		{
			// clear the keymap
			KeyMap.clear();

			// add actions
			for (u32 i = 0; i < count; ++i)
			{
				KeyMap.push_back(map[i]);
			}
		}

		void FpsComponent::setKeyMap(const core::array<SKeyMap> &keymap)
		{
			KeyMap = keymap;
		}

		const core::array<SKeyMap> &FpsComponent::getKeyMap() const
		{
			return KeyMap;
		}

		//! Sets whether vertical movement should be allowed.
		void FpsComponent::setVerticalMovement(bool allow)
		{
			NoVerticalMovement = !allow;
		}

		//! Sets whether the Y axis of the mouse should be inverted.
		void FpsComponent::setInvertMouse(bool invert)
		{
			if (invert)
				MouseYDirection = -1.0f;
			else
				MouseYDirection = 1.0f;
		}

		void FpsComponent::OnAnimate(f32 deltaTime)
		{

			if (!owner || owner->getType() != ESNT_CAMERA)
				return;

			scene::ISceneManager *smgr = owner->getSceneManager();
			video::IVideoDriver *driver = smgr->getVideoDriver();

			ICameraSceneNode *camera = static_cast<ICameraSceneNode *>(owner);

			// If the camera isn't the active camera, and receiving input, then don't process it.
			if (!camera->isInputReceiverEnabled())
			{
				firstInput = true;
				return;
			}

			if (firstInput)
			{
				allKeysUp();
				firstInput = false;
			}
			if (smgr && smgr->getActiveCamera() != camera)
				return;

			core::vector3df moveVector(0, 0, 0);
			core::vector3df forward = GetCameraDirection();
			core::vector3df right = GetCameraRight();
			forward.Y = 0;
			forward.normalize();
			right.normalize();

			// WASD movement
			if (CursorKeys[EKA_MOVE_FORWARD])
				moveVector += forward;
			if (CursorKeys[EKA_MOVE_BACKWARD])
				moveVector -= forward;
			if (CursorKeys[EKA_STRAFE_LEFT])
				moveVector -= right;
			if (CursorKeys[EKA_STRAFE_RIGHT])
				moveVector += right;

			if (moveVector.getLength() > 0)
			{
				moveVector.normalize();
				CameraPosition += moveVector * MoveSpeed * deltaTime;
			}

			core::vector3df cameraTarget = CameraPosition + GetCameraDirection();

			camera->setPosition(CameraPosition);
			camera->setTarget(cameraTarget);
		}

		bool FpsComponent::OnEvent(const SEvent &evt)
		{
			switch (evt.EventType)
			{
			case EET_KEY_INPUT_EVENT:
			{
				for (u32 i = 0; i < KeyMap.size(); ++i)
				{
					if (KeyMap[i].KeyCode == evt.KeyInput.Key)
					{
						CursorKeys[KeyMap[i].Action] = evt.KeyInput.PressedDown;
						return true;
					}
				}
				break;
			}
			case EET_MOUSE_INPUT_EVENT:
			{
				if (evt.MouseInput.Event == EMIE_MOUSE_MOVED)
				{

					if (FirstMouseInput)
					{
						CursorPos = CursorControl->getRelativePosition();

						f32 deltaX = CursorPos.X - LastMousePos.X;
						f32 deltaY = CursorPos.Y - LastMousePos.Y;

						CameraYaw += deltaX * MouseSensitivity;
						CameraPitch += deltaY * MouseSensitivity * MouseYDirection;

						if (CameraPitch > MaxVerticalAngle)
							CameraPitch = MaxVerticalAngle;
						else if (CameraPitch < -MaxVerticalAngle)
							CameraPitch = -MaxVerticalAngle;

						if (CameraYaw >= 360.0f)
							CameraYaw -= 360.0f;
						else if (CameraYaw < 0.0f)
							CameraYaw += 360.0f;
						LastMousePos = CursorPos;
					}

					return true;
				}
				else if (evt.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN)
				{

					LastMousePos = CursorControl->getRelativePosition();
					FirstMouseInput = true;

					CursorControl->setVisible(false);

					return true;
				}
				else if (evt.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
				{
					FirstMouseInput = false;
					CursorControl->setVisible(true);

					return true;
				}
				break;
			}
			default:
				break;
			}
			return false;
		}

		FpsComponent::~FpsComponent()
		{
			os::Printer::log("Destroy FPS Camera Component", ELL_INFORMATION);
			if (CursorControl)
				CursorControl->drop();
		}

		FreeCameraComponent::FreeCameraComponent()
		{
			os::Printer::log("Create Free Camera Component", ELL_INFORMATION);
			MoveSpeed = 5.0f;
			RotateSpeed = 100.0f;
			RollSpeed = 50.0f;
			MouseSensitivity = 100.5f;

			Yaw = 0.0f;
			Pitch = 0.0f;
			Roll = 0.0f;

			Position.set(0, 0, 0);

			MouseCaptured = false;
			FirstMouseInput = true;

			LimitPitch = true;
			MaxPitch = 89.0f;
			LimitRoll = false;
			MaxRoll = 45.0f;

			for (u32 i = 0; i < E6DOF_COUNT; ++i)
				CursorKeys[i] = false;

			KeyMap.push_back(SKeyMap(EKA_MOVE_FORWARD, KEY_KEY_W));
			KeyMap.push_back(SKeyMap(EKA_MOVE_BACKWARD, KEY_KEY_S));
			KeyMap.push_back(SKeyMap(EKA_STRAFE_LEFT, KEY_KEY_A));
			KeyMap.push_back(SKeyMap(EKA_STRAFE_RIGHT, KEY_KEY_D));
			KeyMap.push_back(SKeyMap((EKEY_ACTION)E6DOF_MOVE_UP, KEY_KEY_Q));
			KeyMap.push_back(SKeyMap((EKEY_ACTION)E6DOF_MOVE_DOWN, KEY_KEY_E));
			KeyMap.push_back(SKeyMap((EKEY_ACTION)E6DOF_ROLL_LEFT, KEY_KEY_Z));
			KeyMap.push_back(SKeyMap((EKEY_ACTION)E6DOF_ROLL_RIGHT, KEY_KEY_C));
			KeyMap.push_back(SKeyMap((EKEY_ACTION)E6DOF_BOOST, KEY_LSHIFT));
			KeyMap.push_back(SKeyMap((EKEY_ACTION)E6DOF_SLOW, KEY_LCONTROL));
		}

		FreeCameraComponent::~FreeCameraComponent()
		{
			os::Printer::log("Destroy Free Camera Component", ELL_INFORMATION);
			if (CursorControl)
				CursorControl->drop();
		}

		void FreeCameraComponent::OnReady()
		{
			os::Printer::log("Ready Free Camera Component", ELL_INFORMATION);
			CursorControl = owner->getSceneManager()->getCursorControl();
			CursorControl->grab();
			Position = owner->getPosition();
		}

		void FreeCameraComponent::OnAnimate(f32 deltaTime)
		{
			if (!owner || owner->getType() != ESNT_CAMERA)
				return;

			ICameraSceneNode *camera = static_cast<ICameraSceneNode *>(owner);

			// Multiplicador de velocidade
			f32 speedMultiplier = 1.0f;
			if (CursorKeys[E6DOF_BOOST])
				speedMultiplier = 3.0f;
			if (CursorKeys[E6DOF_SLOW])
				speedMultiplier = 0.3f;

			f32 currentMoveSpeed = MoveSpeed * speedMultiplier * deltaTime;
			f32 currentRollSpeed = RollSpeed * deltaTime;

			core::vector3df forward = getForwardVector();
			core::vector3df right = getRightVector();
			core::vector3df up = getUpVector();

			// Movimento translacional (6DOF completo)
			core::vector3df moveVector(0, 0, 0);

			if (CursorKeys[EKA_MOVE_FORWARD])
				moveVector += forward * currentMoveSpeed;
			if (CursorKeys[EKA_MOVE_BACKWARD])
				moveVector -= forward * currentMoveSpeed;
			if (CursorKeys[EKA_STRAFE_RIGHT])
				moveVector += right * currentMoveSpeed;
			if (CursorKeys[EKA_STRAFE_LEFT])
				moveVector -= right * currentMoveSpeed;
			if (CursorKeys[E6DOF_MOVE_UP])
				moveVector += up * currentMoveSpeed;
			if (CursorKeys[E6DOF_MOVE_DOWN])
				moveVector -= up * currentMoveSpeed;

			Position += moveVector;

			// Movimento rotacional - Roll com teclado
			if (CursorKeys[E6DOF_ROLL_LEFT])
				Roll += currentRollSpeed;
			if (CursorKeys[E6DOF_ROLL_RIGHT])
				Roll -= currentRollSpeed;

			// Aplicar limites
			if (LimitPitch)
			{
				if (Pitch > MaxPitch)
					Pitch = MaxPitch;
				if (Pitch < -MaxPitch)
					Pitch = -MaxPitch;
			}

			if (LimitRoll)
			{
				if (Roll > MaxRoll)
					Roll = MaxRoll;
				if (Roll < -MaxRoll)
					Roll = -MaxRoll;
			}

			// Normalizar Yaw
			while (Yaw >= 360.0f)
				Yaw -= 360.0f;
			while (Yaw < 0.0f)
				Yaw += 360.0f;

 
	 
			camera->setPosition(Position);
			camera->setTarget(Position + forward);
			camera->setUpVector(up);
		

			}
			
		void FreeCameraComponent::Render()
		{
			// if (!owner || owner->getType() != ESNT_CAMERA)
			// 	return;

			// ICameraSceneNode *camera = static_cast<ICameraSceneNode *>(owner);

			// core::vector3df forward = getForwardVector();
			// core::vector3df right = getRightVector();
			// core::vector3df up = getUpVector();
			// camera->setPosition(Position);
			// camera->setTarget(Position + forward);
			// camera->setUpVector(up);
			// camera->updateMatrices();
        }

		bool FreeCameraComponent::OnEvent(const SEvent &event)
		{
			switch (event.EventType)
			{
			case EET_KEY_INPUT_EVENT:
			{
				// Processar keymap
				for (u32 i = 0; i < KeyMap.size(); ++i)
				{
					if (KeyMap[i].KeyCode == event.KeyInput.Key)
					{
						u32 action = KeyMap[i].Action;
						if (action < E6DOF_COUNT)
							CursorKeys[action] = event.KeyInput.PressedDown;
						return true;
					}
				}

				// Teclas especiais
				if (event.KeyInput.Key == KEY_SPACE && event.KeyInput.PressedDown)
				{
					// Reset orientação
					resetOrientation();
					return true;
				}

				break;
			}

			case EET_MOUSE_INPUT_EVENT:
			{
				if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN)
				{
					// Capturar mouse com botão direito
					MouseCaptured = true;
					LastMousePos = CursorControl->getRelativePosition();
					FirstMouseInput = true;
					CursorControl->setVisible(false);
					return true;
				}
				else if (event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP)
				{
					// Liberar mouse
					MouseCaptured = false;
					CursorControl->setVisible(true);
					return true;
				}
				else if (event.MouseInput.Event == EMIE_MOUSE_MOVED && MouseCaptured)
				{
					if (!FirstMouseInput)
					{
						core::position2d<f32> currentPos = CursorControl->getRelativePosition();

						f32 deltaX = currentPos.X - LastMousePos.X;
						f32 deltaY = currentPos.Y - LastMousePos.Y;

						// Rotação com mouse
						Yaw += deltaX * MouseSensitivity;
						Pitch += deltaY * MouseSensitivity;

						LastMousePos = currentPos;
					}
					else
					{
						LastMousePos = CursorControl->getRelativePosition();
						FirstMouseInput = false;
					}
					return true;
				}

				break;
			}

			default:
				break;
			}

			return false;
		}

   

        // Vetores direcionais baseados na orientação
		core::vector3df FreeCameraComponent::getForwardVector() const
		{
			f32 yawRad = Yaw * core::DEGTORAD;
			f32 pitchRad = Pitch * core::DEGTORAD;

			core::vector3df forward;
			forward.X = cos(pitchRad) * sin(yawRad);
			forward.Y = -sin(pitchRad);
			forward.Z = cos(pitchRad) * cos(yawRad);

			return forward.normalize();
		}

		core::vector3df FreeCameraComponent::getRightVector() const
		{
			f32 yawRad = (Yaw + 90.0f) * core::DEGTORAD;
			f32 rollRad = Roll * core::DEGTORAD;

			core::vector3df right;
			right.X = sin(yawRad) * cos(rollRad);
			right.Y = sin(rollRad);
			right.Z = cos(yawRad) * cos(rollRad);

			return right.normalize();
		}

		core::vector3df FreeCameraComponent::getUpVector() const
		{
			  core::vector3df f = getForwardVector();
    		core::vector3df r = getRightVector();
    		return f.crossProduct(r).normalize();
		}

		core::vector3df FreeCameraComponent::getLeftVector() const
		{

			f32 yawRad = (Yaw - 90.0f) * core::DEGTORAD;
			f32 rollRad = Roll * core::DEGTORAD;

			core::vector3df left;
			left.X = sin(yawRad) * cos(rollRad);
			left.Y = sin(rollRad);
			left.Z = cos(yawRad) * cos(rollRad);

			return left.normalize();
		}

		void FreeCameraComponent::allKeysUp()
		{
			for (u32 i = 0; i < E6DOF_COUNT; ++i)
				CursorKeys[i] = false;
		}

		//! constructor
		CCameraSceneNode::CCameraSceneNode(ISceneNode *parent, ISceneManager *mgr, s32 id,
										   const core::vector3df &position, const core::vector3df &lookat)
			: ICameraSceneNode(parent, mgr, id, position),
			  Target(lookat), UpVector(0.0f, 1.0f, 0.0f), ZNear(1.0f), ZFar(3000.0f),
			  InputReceiverEnabled(true), TargetAndRotationAreBound(false)
		{
#ifdef _DEBUG
			setDebugName("CCameraSceneNode");
#endif

			// set default projection
			Fovy = core::PI / 2.5f; // Field of view, in radians.

			const video::IVideoDriver *const d = mgr ? mgr->getVideoDriver() : 0;
			if (d)
				Aspect = (f32)d->getCurrentRenderTargetSize().Width /
						 (f32)d->getCurrentRenderTargetSize().Height;
			else
				Aspect = 4.0f / 3.0f; // Aspect ratio.

			recalculateProjectionMatrix();
			recalculateViewArea();
			ViewArea.setFarNearDistance(ZFar - ZNear);
		}

		//! Disables or enables the camera to get key or mouse inputs.
		void CCameraSceneNode::setInputReceiverEnabled(bool enabled)
		{
			InputReceiverEnabled = enabled;
		}

		//! Returns if the input receiver of the camera is currently enabled.
		bool CCameraSceneNode::isInputReceiverEnabled() const
		{
			_IRR_IMPLEMENT_MANAGED_MARSHALLING_BUGFIX;
			return InputReceiverEnabled;
		}

		//! Sets the projection matrix of the camera.
		/** The core::matrix4 class has some methods
		to build a projection matrix. e.g: core::matrix4::buildProjectionMatrixPerspectiveFovLH
		\param projection: The new projection matrix of the camera. */
		void CCameraSceneNode::setProjectionMatrix(const core::matrix4 &projection, bool isOrthogonal)
		{
			IsOrthogonal = isOrthogonal;
			ViewArea.getTransform(video::ETS_PROJECTION) = projection;
		}

		//! Gets the current projection matrix of the camera
		//! \return Returns the current projection matrix of the camera.
		const core::matrix4 &CCameraSceneNode::getProjectionMatrix() const
		{
			return ViewArea.getTransform(video::ETS_PROJECTION);
		}

		//! Gets the current view matrix of the camera
		//! \return Returns the current view matrix of the camera.
		const core::matrix4 &CCameraSceneNode::getViewMatrix() const
		{
			return ViewArea.getTransform(video::ETS_VIEW);
		}

		//! Sets a custom view matrix affector. The matrix passed here, will be
		//! multiplied with the view matrix when it gets updated.
		//! This allows for custom camera setups like, for example, a reflection camera.
		/** \param affector: The affector matrix. */
		void CCameraSceneNode::setViewMatrixAffector(const core::matrix4 &affector)
		{
			Affector = affector;
		}

		//! Gets the custom view matrix affector.
		const core::matrix4 &CCameraSceneNode::getViewMatrixAffector() const
		{
			return Affector;
		}

		//! It is possible to send mouse and key events to the camera. Most cameras
		//! may ignore this input, but camera scene nodes which are created for
		//! example with scene::ISceneManager::addMayaCameraSceneNode or
		//! scene::ISceneManager::addFPSCameraSceneNode, may want to get this input
		//! for changing their position, look at target or whatever.
		bool CCameraSceneNode::OnEvent(const SEvent &event)
		{

			if (!InputReceiverEnabled)
				return false;

			if (ISceneNode::OnEvent(event))
				return true;

			// if nobody processed the event, return false
			return false;
		}

		//! sets the look at target of the camera
		//! \param pos: Look at target of the camera.
		void CCameraSceneNode::setTarget(const core::vector3df &pos)
		{
			Target = pos;

			if (TargetAndRotationAreBound)
			{
				const core::vector3df toTarget = Target - getAbsolutePosition();
				ISceneNode::setRotation(toTarget.getHorizontalAngle());
			}
		}

		//! Sets the rotation of the node.
		/** This only modifies the relative rotation of the node.
		If the camera's target and rotation are bound ( @see bindTargetAndRotation() )
		then calling this will also change the camera's target to match the rotation.
		\param rotation New rotation of the node in degrees. */
		void CCameraSceneNode::setRotation(const core::vector3df &rotation)
		{
			if (TargetAndRotationAreBound)
				Target = getAbsolutePosition() + rotation.rotationToDirection();

			ISceneNode::setRotation(rotation);
		}

		//! Gets the current look at target of the camera
		//! \return Returns the current look at target of the camera
		const core::vector3df &CCameraSceneNode::getTarget() const
		{
			return Target;
		}

		//! sets the up vector of the camera
		//! \param pos: New upvector of the camera.
		void CCameraSceneNode::setUpVector(const core::vector3df &pos)
		{
			UpVector = pos;
		}

		//! Gets the up vector of the camera.
		//! \return Returns the up vector of the camera.
		const core::vector3df &CCameraSceneNode::getUpVector() const
		{
			return UpVector;
		}

		f32 CCameraSceneNode::getNearValue() const
		{
			return ZNear;
		}

		f32 CCameraSceneNode::getFarValue() const
		{
			return ZFar;
		}

		f32 CCameraSceneNode::getAspectRatio() const
		{
			return Aspect;
		}

		f32 CCameraSceneNode::getFOV() const
		{
			return Fovy;
		}

		void CCameraSceneNode::setNearValue(f32 f)
		{
			ZNear = f;
			recalculateProjectionMatrix();
			ViewArea.setFarNearDistance(ZFar - ZNear);
		}

		void CCameraSceneNode::setFarValue(f32 f)
		{
			ZFar = f;
			recalculateProjectionMatrix();
			ViewArea.setFarNearDistance(ZFar - ZNear);
		}

		void CCameraSceneNode::setAspectRatio(f32 f)
		{
			Aspect = f;
			recalculateProjectionMatrix();
		}

		void CCameraSceneNode::setFOV(f32 f)
		{
			Fovy = f;
			recalculateProjectionMatrix();
		}

		void CCameraSceneNode::recalculateProjectionMatrix()
		{
			ViewArea.getTransform(video::ETS_PROJECTION).buildProjectionMatrixPerspectiveFovLH(Fovy, Aspect, ZNear, ZFar);
		}

		//! prerender
		void CCameraSceneNode::OnRegisterSceneNode()
		{
			if (SceneManager->getActiveCamera() == this)
				SceneManager->registerNodeForRendering(this, ESNRP_CAMERA);

			ISceneNode::OnRegisterSceneNode();
		}

		//! render
		void CCameraSceneNode::render()
		{

	 
			updateMatrices();

			video::IVideoDriver *driver = SceneManager->getVideoDriver();
			if (driver)
			{
				driver->setTransform(video::ETS_PROJECTION, ViewArea.getTransform(video::ETS_PROJECTION));
				driver->setTransform(video::ETS_VIEW, ViewArea.getTransform(video::ETS_VIEW));
			}
		}

		//! update
		void CCameraSceneNode::updateMatrices()
		{
			core::vector3df pos = getAbsolutePosition();
			core::vector3df tgtv = Target - pos;
			tgtv.normalize();

			// if upvector and vector to the target are the same, we have a
			// problem. so solve this problem:
			core::vector3df up = UpVector;
			up.normalize();

			f32 dp = tgtv.dotProduct(up);

			if (core::equals(core::abs_<f32>(dp), 1.f))
			{
				up.X += 0.5f;
			}

			ViewArea.getTransform(video::ETS_VIEW).buildCameraLookAtMatrixLH(pos, Target, up);
			ViewArea.getTransform(video::ETS_VIEW) *= Affector;
			recalculateViewArea();
		}

		//! returns the axis aligned bounding box of this node
		const core::aabbox3d<f32> &CCameraSceneNode::getBoundingBox() const
		{
			return ViewArea.getBoundingBox();
		}

		//! returns the view frustum. needed sometimes by bsp or lod render nodes.
		const SViewFrustum *CCameraSceneNode::getViewFrustum() const
		{
			return &ViewArea;
		}

		void CCameraSceneNode::recalculateViewArea()
		{
			ViewArea.cameraPosition = getAbsolutePosition();

			core::matrix4 m(core::matrix4::EM4CONST_NOTHING);
			m.setbyproduct_nocheck(ViewArea.getTransform(video::ETS_PROJECTION),
								   ViewArea.getTransform(video::ETS_VIEW));
			ViewArea.setFrom(m);
		}

	 

		//! Set the binding between the camera's rotation adn target.
		void CCameraSceneNode::bindTargetAndRotation(bool bound)
		{
			TargetAndRotationAreBound = bound;
		}

		//! Gets the binding between the camera's rotation and target.
		bool CCameraSceneNode::getTargetAndRotationBinding(void) const
		{
			return TargetAndRotationAreBound;
		}

		//! Creates a clone of this scene node and its children.
		ISceneNode *CCameraSceneNode::clone(ISceneNode *newParent, ISceneManager *newManager)
		{
			ICameraSceneNode::clone(newParent, newManager);

			if (!newParent)
				newParent = Parent;
			if (!newManager)
				newManager = SceneManager;

			CCameraSceneNode *nb = new CCameraSceneNode(newParent,
														newManager, ID, RelativeTranslation, Target);

			nb->ISceneNode::cloneMembers(this, newManager);
			nb->ICameraSceneNode::cloneMembers(this);

			nb->Target = Target;
			nb->UpVector = UpVector;
			nb->Fovy = Fovy;
			nb->Aspect = Aspect;
			nb->ZNear = ZNear;
			nb->ZFar = ZFar;
			nb->ViewArea = ViewArea;
			nb->Affector = Affector;
			nb->InputReceiverEnabled = InputReceiverEnabled;
			nb->TargetAndRotationAreBound = TargetAndRotationAreBound;

			if (newParent)
				nb->drop();
			return nb;
		}

	} // end namespace
} // end namespace
