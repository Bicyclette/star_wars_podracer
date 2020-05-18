/**
 * \file
 * It looks like Ben Quadinaros has an engine trouble also !
 * \author Mathias Velo
 */

#include "power.hpp"

Power::Power(std::vector<glm::vec3> co_left, std::vector<glm::vec3> co_right) :
	jitter1(0.0625f),
	jitter2(0.125f),
	thickness(0.05f),
	rng(std::chrono::steady_clock::now().time_since_epoch().count()),
	dis1(0.01f, 0.99f),
	dis2(-jitter1, jitter1),
	dis3(-jitter2, jitter2)
{
    // connector_left bounding box
    float left_x_min = 0.0f;
    float left_y_min = 0.0f;
    float left_z_min = 0.0f;
    float left_x_max = 0.0f;
    float left_y_max = 0.0f;
    float left_z_max = 0.0f;
        
    glm::vec3 v = co_left.at(0);
    left_x_min = v.x;
    left_x_max = v.x;
        
    left_y_min = v.y;
    left_y_max = v.y;
        
    left_z_min = v.z;
    left_z_max = v.z;

    for(int i = 1; i < co_left.size(); i++)
    {
        v = co_left.at(i);
        if(v.x <= left_x_min)
            left_x_min = v.x;
        else if(v.x >= left_x_max)
            left_x_max = v.x;
        if(v.y <= left_y_min)
            left_y_min = v.y;
        else if(v.y >= left_y_max)
            left_y_max = v.y;
        if(v.z <= left_z_min)
            left_z_min = v.z;
        else if(v.z >= left_z_max)
            left_z_max = v.z;
    }

    // connector_right bounding box
    float right_x_min = 0.0f;
    float right_y_min = 0.0f;
    float right_z_min = 0.0f;
    float right_x_max = 0.0f;
    float right_y_max = 0.0f;
    float right_z_max = 0.0f;
    
    v = co_right.at(0);
    right_x_min = v.x;
    right_x_max = v.x;
        
    right_y_min = v.y;
    right_y_max = v.y;
        
    right_z_min = v.z;
    right_z_max = v.z;

    for(int i = 1; i < co_right.size(); i++)
    {
        glm::vec3 v = co_right.at(i);
        if(v.x <= right_x_min)
            right_x_min = v.x;
        else if(v.x >= right_x_max)
            right_x_max = v.x;
        if(v.y <= right_y_min)
            right_y_min = v.y;
        else if(v.y >= right_y_max)
            right_y_max = v.y;
        if(v.z <= right_z_min)
            right_z_min = v.z;
        else if(v.z >= right_z_max)
            right_z_max = v.z;
    }

    // center left connector
    glm::vec3 center_left_co = glm::vec3(left_x_min, (left_y_max + left_y_min) / 2.0f, (left_z_max + left_z_min) / 2.0f);
    // center right connector
    glm::vec3 center_right_co = glm::vec3(right_x_max, (right_y_max + right_y_min) / 2.0f, (right_z_max + right_z_min) / 2.0f);
	
    start = new BoltNode(center_left_co, glm::vec2(0.0f, 0.0f));
	end = new BoltNode(center_right_co, glm::vec2(1.0f, 0.0f));

	// VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_BOLT_NODES * sizeof(BoltNode), nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BoltNode), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BoltNode), (void*)(offsetof(BoltNode, texCoords)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Unbind VAO
	glBindVertexArray(0);
}

int cmp(const void* a, const void* b)
{
	float arg1 = *static_cast<const float*>(a);
	float arg2 = *static_cast<const float*>(b);
	if(arg1 < arg2)
		return -1;
	else if(arg1 > arg2)
		return 1;
	else
		return 0;
}

float Power::get_random(int choice)
{
	if(choice == 1)
		return dis1(rng);
	else if(choice == 2)
		return dis2(rng);
	else if(choice == 3)
		return dis3(rng);
}

void Power::create_bolt(int jitter)
{
	#pragma omp for
	offset_x.clear();
	for(int i = 0; i < 12 - 2; i++)
	{
		offset_x.push_back(get_random(1));
	}
	std::qsort(offset_x.data(), offset_x.size(), sizeof(float), cmp);
	float diff_x = end->position.x - start->position.x;

	// create set of triangles	
	float randY, randZ;
	randY = get_random(jitter);
	randZ = get_random(jitter);
	
	BoltNode zero(start->position, start->texCoords);
	
	BoltNode first(
			zero.position + glm::vec3(offset_x.at(0) * diff_x, randY, randZ),
			glm::vec2(0.0f, 0.0f)
			);
	
	BoltNode second(
			zero.position + glm::vec3(0.0f, thickness, 0.0f),
			glm::vec2(0.0f, 1.0f)
			);
	
	BoltNode third(
			first.position + glm::vec3(0.0f, thickness, 0.0f),
			glm::vec2(0.0f, 1.0f)
			);

	bolt.clear();
	bolt.push_back(zero);
	bolt.push_back(first);
	bolt.push_back(second);
	bolt.push_back(second);
	bolt.push_back(first);
	bolt.push_back(third);

	for(int i = 1; i < 10; i++)
	{
		randY = get_random(jitter);
		randZ = get_random(jitter);

		zero.position = first.position;
		zero.texCoords = first.texCoords;
	
		first.position = start->position + glm::vec3(offset_x.at(i) * diff_x, randY, randZ);
		first.texCoords = glm::vec2(0.0f, 0.0f);
	
		second.position = third.position;
		second.texCoords = glm::vec2(0.0f, 1.0f);
	
		third.position = first.position + glm::vec3(0.0f, thickness, 0.0f);
		third.texCoords = glm::vec2(0.0f, 1.0f);

		bolt.push_back(zero);
		bolt.push_back(first);
		bolt.push_back(second);
		bolt.push_back(second);
		bolt.push_back(first);
		bolt.push_back(third);
	}
	randY = get_random(jitter);
	randZ = get_random(jitter);
	
	zero.position = first.position;
	zero.texCoords = first.texCoords;
	
	first.position = end->position;
	first.texCoords = glm::vec2(0.0f, 0.0f);
	
	second.position = third.position;
	second.texCoords = glm::vec2(0.0f, 1.0f);
	
	third.position = first.position + glm::vec3(0.0f, thickness, 0.0f);
	third.texCoords = glm::vec2(0.0f, 1.0f);

	bolt.push_back(zero);
	bolt.push_back(first);
	bolt.push_back(second);
	bolt.push_back(second);
	bolt.push_back(first);
	bolt.push_back(third);
}

void Power::draw(Shader* power_shader)
{
	// create 3 bolts
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	for(int i = 0; i < 3; i++)
	{
		// create a bolt
		if(i == 0)
			create_bolt(i + 2);
		else
			create_bolt(i + 1);

		// draw
		glBufferSubData(GL_ARRAY_BUFFER, 0, bolt.size() * sizeof(BoltNode), bolt.data());
		glDrawArrays(GL_TRIANGLES, 0, bolt.size());
	}

	glBindVertexArray(0);
}
