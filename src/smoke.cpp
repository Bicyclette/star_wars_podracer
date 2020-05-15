#include "smoke.hpp"

Smoke::Smoke(std::vector<glm::vec3> sources, glm::vec3 sources_dir) :
	velocity(0.00125f),
	dissolve(0.5f),
	jitter(0.05f),
	dis(0.0f, 0.25f)
{
	gen.seed(rd());
	// smoke sources
	if(sources.size() == 3)
	{
		s1 = sources.at(0);
		s2 = sources.at(1);
		s3 = sources.at(2);
	}
	else
	{
		std::cerr << "Error, invalid smoke sources count !" << std::endl;
	}

	particles.push_back(Particle(s1, sources_dir));
	particles.push_back(Particle(s2, sources_dir));
	particles.push_back(Particle(s3, sources_dir));

	// smoke attributes
	jitter = 0.05f;

	// smoke particles in VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(Particle), particles.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, lifeTime)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, direction)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// unbind VAO
	glBindVertexArray(0);
}

void Smoke::set_init_dir(glm::vec3 dir)
{
	direction = dir;
}

void Smoke::draw(double delta, Shader* smoke_shader)
{
	// draw particles
	glBindVertexArray(VAO);
	int count = particles.size();
	for(int p = count; p > 3; p -= 3)
	{
		glDrawArrays(GL_POINTS, p - 3, 3);
	}

	// update particles array
	int particles_count = particles.size();
	auto it = particles.begin();

	float fps = static_cast<float>(1.0/delta);
	float ratio = 60.0f / fps;
	float scale_lifeTime = fps / 60.0f;

	for(int i = 0; i < particles.size(); i++)
	{
		Particle& p = particles.at(i);
		float updated_lifeTime = p.lifeTime + (static_cast<float>(delta) * scale_lifeTime);
		if(updated_lifeTime >= dissolve)
		{
			particles.erase(it + i);
		}
		else
		{
			p.lifeTime = updated_lifeTime;
			p.position += velocity * ratio * p.direction;
			float jitter_x = dis(gen);
			float jitter_y = dis(gen);
			float jitter_z = dis(gen);
			p.position += glm::vec3(jitter_x * p.direction.x, jitter_y * p.direction.y, jitter_z * p.direction.z);
		}
	}

	particles.push_back(Particle(s1, direction));
	particles.push_back(Particle(s2, direction));
	particles.push_back(Particle(s3, direction));
	
	// update smoke's VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * sizeof(Particle), particles.data());

	// unbind VAO
	glBindVertexArray(0);
}

