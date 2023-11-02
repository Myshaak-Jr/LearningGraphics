#pragma once

class IMaterial {
public:
	virtual ~IMaterial() = default;

	
};

class SingleColorMaterial : public IMaterial {

};

class TextureMaterial : public IMaterial {

};

// TODO
//class ShellTexturingMaterial : public IMaterial {
//
//};

