/*

所有感受器的基类
定义了激活/不激活状态，以及激活信息

*/

#pragma once

#include "Config.h"
#include "ObjectManagement.h"
#include "Object.h"
#include "MapNote.h"

class Receptor : public Object
{
	//变量
protected:
	enum ReceptorState
	{
		release = 0,
		activation
	} state, prev_state;

	std::string activation_message, release_message;
	std::vector<unsigned int>	effectors_list;

	unsigned short delay_counter;

	//函数
public:
	Receptor(int i_x, int i_y, ObjectManagement* i_object_manager);
	Receptor(const MapNote::ReceptorAndEffectors& receptor_info, ObjectManagement* i_object_manager);
	~Receptor();

	//编码
	virtual const unsigned short encode() const override;
	//解码
	virtual void decode(const unsigned short i_eigen_code) override;

	void getStateByPostalCode(unsigned int& i_postal_code) const;

protected:
	virtual void f_activation();
	virtual void f_release();
};