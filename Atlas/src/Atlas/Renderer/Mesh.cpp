#include "atlpch.h"

#include "Mesh.h"
#include "Renderer.h"

#include <thread>
#include <filesystem>

namespace std {
	template <>
	struct hash<glm::vec3>
	{
		std::size_t operator()(const glm::vec3& k) const
		{
			using std::size_t;
			using std::hash;
			using std::string;
			return ((hash<float>()(k.x)
				^ (hash<float>()(k.y) << 1)) >> 1)
				^ (hash<float>()(k.z) << 1);

		}
	};
}

namespace Atlas {
	std::string Utils::DisplayEnumToString(DisplayMode type)
	{
		switch (type)
		{
		case DisplayMode::SOLID:
			return "Solid";

		case DisplayMode::NORMAL:
			return "Normal";
		}

		ATL_CORE_ERROR("Unknown DisplayMode");
		return "";
	}
}

namespace Atlas {

	void Mesh::InitTextures()
	{
		uint32_t whiteTextureData = 0xFFFFFFFF;
		Ref<Texture2D> whiteTexture = Texture2D::Create(1, 1);
		whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		Mesh::m_Textures[(int)Utils::TextureType::DIFFUSE] = whiteTexture;
		Mesh::m_Textures[(int)Utils::TextureType::SPECULAR] = whiteTexture;
		Mesh::m_Textures[(int)Utils::TextureType::NORMAL] = whiteTexture;
	}

	Mesh::Mesh()
	{
		m_FlatVertexTriangles = new VertexTriangle[0];
		m_Indices = new uint32_t[0];
		m_VertexArray = VertexArray::Create();

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(0);
		vertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float2, "a_TextureCoord" },
			});

		m_VertexArray->AddVertexBuffer(vertexBuffer);
		m_VertexArray->SetIndexBuffer(IndexBuffer::Create(0));

		InitTextures();
	}

	Mesh::Mesh(const std::string& path)
		: m_FilePath(path)
	{
		InitTextures();
		Load(path);

	}

	Mesh::~Mesh()
	{
		delete[] m_FlatVertexTriangles;
		delete[] m_SmoothVertexTriangles;
		delete[] m_Indices;
	}

	void Mesh::Load(const std::string& path)
	{
		std::filesystem::path filepath = path;
		m_Name = filepath.stem().string();

		ATL_PROFILE_FUNCTION();

		std::vector<glm::vec3> vertexPos;
		std::vector<glm::vec3> vertexNormals;
		std::vector<glm::vec2> vertexTexCoords;
		std::vector<std::tuple<int, int, int>> trianglePosIndices;
		std::vector<std::tuple<int, int, int>> triangleTexCoordIndices;
		std::vector<std::tuple<int, int, int>> triangleNormalIndices;

		bool hasVertexNormals = false;

		std::fstream file;
		{
			ATL_PROFILE_SCOPE("read obj file");
			file = std::fstream(path);
		}

		std::string line;
		std::string numbers;

		while (std::getline(file, line))
		{

			if (line[0] == 'v')
			{

				if (line[1] == 't')
				{
					m_TextureCoordCount++;
					m_HasTexCoords = true;
					numbers = line.substr(3, line.size() - 1);

					char num[2][16]{};

					short indx = 0;
					short numCount = 0;
					for (char c : numbers)
					{
						if (c == ' ')
						{
							indx = 0;
							numCount += 1;
							ATL_CORE_ASSERT(numCount <= 1, "invalid format for Texture Coords!");
						}

						if (indx >= 16) break;
						num[numCount][indx] = c;

						indx++;
					}

					vertexTexCoords.push_back({ (float)std::atof(num[0]), (float)std::atof(num[1]) });

				}
				else if (line[1] == 'n')
				{
					numbers = line.substr(3, line.size() - 1);
					hasVertexNormals = true;
					char num[3][16]{};

					short indx = 0;
					short numCount = 0;
					for (char c : numbers)
					{
						if (c == ' ')
						{
							indx = 0;
							numCount += 1;
							if (numCount > 2) break;
						}

						if (indx >= 16) break;
						num[numCount][indx] = c;

						indx++;
					}

					vertexNormals.push_back({ (float)std::atof(num[0]), (float)std::atof(num[1]), std::atof(num[2]) });
				}
				else
				{
					m_VertexCount++;
					numbers = line.substr(2, line.size() - 1);
					char num[3][16]{};

					short indx = 0;
					short numCount = 0;
					for (char c : numbers)
					{
						if (c == ' ')
						{
							indx = 0;
							numCount += 1;
							if (numCount > 2) break;
						}

						if (indx >= 16) break;
						num[numCount][indx] = c;

						indx++;
					}

					vertexPos.push_back({ (float)std::atof(num[0]), (float)std::atof(num[1]), std::atof(num[2]) });

				}
			}

			if (line[0] == 'f')
			{
				m_TriangleCount++;
				numbers = line.substr(2, line.size() - 1);

				char vertexIndecis[3][16]{};
				char texCoordIndecis[3][16]{};
				char vertexNormalIndecis[3][16]{};

				short vertexIndx = 0;
				short texCoordIndx = 0;
				short vertexNormalIndx = 0;
				short indxCount = 0;

				short indxType = 0;

				bool skip = false;

				for (char c : numbers)
				{
					if (!skip)
					{
						if (c == '/')
						{
							indxType++;
							if (indxType > 2)
							{
								skip = true;
							}

							continue;
						}

						if (indxType == 0)
						{
							vertexIndecis[indxCount][vertexIndx] = c;
							vertexIndx++;
						}
						else if (indxType == 1)
						{
							texCoordIndecis[indxCount][texCoordIndx] = c;
							texCoordIndx++;
						}
						else if (indxType == 2)
						{
							vertexNormalIndecis[indxCount][vertexNormalIndx] = c;
							vertexNormalIndx++;
						}
					}

					if (skip && c == ' ') skip = false;
					
					if (c == ' ')
					{
						indxCount++;
						vertexIndx = 0;
						vertexNormalIndx = 0;
						texCoordIndx = 0;
						indxType = 0;
					}
				}

				trianglePosIndices.push_back({ (int)std::atof(vertexIndecis[0]) - 1, (int)std::atof(vertexIndecis[1]) - 1, (int)std::atof(vertexIndecis[2]) - 1 });

				if (m_HasTexCoords) triangleTexCoordIndices.push_back({ (int)std::atof(texCoordIndecis[0]) - 1, (int)std::atof(texCoordIndecis[1]) - 1, (int)std::atof(texCoordIndecis[2]) - 1 });
				if (hasVertexNormals) triangleNormalIndices.push_back({ (int)std::atof(vertexNormalIndecis[0]) - 1, (int)std::atof(vertexNormalIndecis[1]) - 1, (int)std::atof(vertexNormalIndecis[2]) - 1 });
			}
		}

		m_FlatVertexTriangles = new VertexTriangle[m_TriangleCount];
		m_SmoothVertexTriangles = new VertexTriangle[m_TriangleCount];

		m_Indices = new uint32_t[m_TriangleCount * 3];

		VertexTriangle* vertexTrianglePtr = m_FlatVertexTriangles;
		uint32_t* indexPtr = m_Indices;

		file.close();

		{
			ATL_PROFILE_SCOPE("put vertex into buffer");

			for (uint32_t i = 0; i < m_TriangleCount; i++)
			{
				auto& posIndx = trianglePosIndices.at(i);

				auto& [ix, iy, iz] = posIndx;
				auto& p0 = vertexPos.at(ix);
				auto& p1 = vertexPos.at(iy);
				auto& p2 = vertexPos.at(iz);

				vertexTrianglePtr->Vertices[0].Position = p0;
				vertexTrianglePtr->Vertices[1].Position = p1;
				vertexTrianglePtr->Vertices[2].Position = p2;

				if (m_HasTexCoords)
				{
					auto& texCoordIndx = triangleTexCoordIndices.at(i);
					auto& [ivx, ivy, ivz] = texCoordIndx;
					auto& t0 = vertexTexCoords.at(ivx);
					auto& t1 = vertexTexCoords.at(ivy);
					auto& t2 = vertexTexCoords.at(ivz);

					vertexTrianglePtr->Vertices[0].TextureCoord = t0;
					vertexTrianglePtr->Vertices[1].TextureCoord = t1;
					vertexTrianglePtr->Vertices[2].TextureCoord = t2;
				}

				if (hasVertexNormals)
				{
					auto& normalIndx = triangleNormalIndices.at(i);
					auto& [inx, iny, inz] = normalIndx;
					auto& n0 = vertexNormals.at(inx);
					auto& n1 = vertexNormals.at(iny);
					auto& n2 = vertexNormals.at(inz);

					vertexTrianglePtr->Vertices[0].Normal = n0;
					vertexTrianglePtr->Vertices[1].Normal = n1;
					vertexTrianglePtr->Vertices[2].Normal = n2;
				}

				m_Indices[3 * i + 0] = 3 * i + 0;
				m_Indices[3 * i + 1] = 3 * i + 1;
				m_Indices[3 * i + 2] = 3 * i + 2;

				vertexTrianglePtr++;
			}
		}

		//TODO: import normals
		//if (!hasVertexNormals) CalculateNormals();

		for (uint32_t i = 0; i < m_TriangleCount; i++) m_SmoothVertexTriangles[i] = m_FlatVertexTriangles[i];

		CalculateNormals(m_FlatVertexTriangles, false);
		CalculateNormals(m_SmoothVertexTriangles, true);

		Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(m_TriangleCount * 3 * sizeof(Vertex));
		vertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normal" },
				{ ShaderDataType::Float2, "a_TextureCoord" },
			});
		vertexBuffer->SetData(m_SmoothShading ? m_SmoothVertexTriangles : m_FlatVertexTriangles, m_TriangleCount * 3 * sizeof(Vertex));

		Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(m_TriangleCount * 3 * sizeof(uint32_t));

		m_VertexArray = VertexArray::Create();
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		m_VertexArray->SetIndexBuffer(indexBuffer);
		indexBuffer->SetData(m_Indices, sizeof(uint32_t), m_TriangleCount * 3);
	}

	void Mesh::Invalidate()
	{
		m_VertexArray->GetVertexBuffer()->SetData(m_SmoothShading ? m_SmoothVertexTriangles : m_FlatVertexTriangles, m_TriangleCount * 3 * sizeof(Vertex));
		m_VertexArray->GetIndexBuffer()->SetData(m_Indices, sizeof(uint32_t), m_TriangleCount * 3);
	}



	void Mesh::CalculateNormals(VertexTriangle* vertexTriangles, bool shading)
	{
		ATL_PROFILE_FUNCTION();

		std::unordered_map<glm::vec3, int> repetitionCount;
		std::unordered_map<glm::vec3, glm::vec3> normalMap;

		for (uint32_t i = 0; i < m_TriangleCount; i++)
		{
			VertexTriangle& tri = vertexTriangles[i];

			Vertex& v0 = tri.Vertices[0];
			Vertex& v1 = tri.Vertices[1];
			Vertex& v2 = tri.Vertices[2];

			glm::vec3 vec1 = v1.Position - v0.Position;
			glm::vec3 vec2 = v2.Position - v0.Position;

			glm::vec3 normal = glm::normalize(glm::cross(vec1, vec2));

			if (!shading)
			{
				v0.Normal = normal;
				v1.Normal = normal;
				v2.Normal = normal;
			}
			else 
			{
				normalMap[v0.Position] += normal;
				normalMap[v1.Position] += normal;
				normalMap[v2.Position] += normal;

				repetitionCount[v0.Position] += 1;
				repetitionCount[v1.Position] += 1;
				repetitionCount[v2.Position] += 1;
			}
		}

		if (shading)
		{
			for (uint32_t i = 0; i < m_TriangleCount; i++)
			{
				VertexTriangle& tri = vertexTriangles[i];

				Vertex& v0 = tri.Vertices[0];
				Vertex& v1 = tri.Vertices[1];
				Vertex& v2 = tri.Vertices[2];

				v0.Normal = normalMap[v0.Position] / (float)repetitionCount[v0.Position];
				v1.Normal = normalMap[v1.Position] / (float)repetitionCount[v1.Position];
				v2.Normal = normalMap[v2.Position] / (float)repetitionCount[v2.Position];
			}
		}
	}

	void Mesh::RecalculateNormals()
	{
		CalculateNormals(m_FlatVertexTriangles, false);
		CalculateNormals(m_SmoothVertexTriangles, true);
		Invalidate();
	}

	void Mesh::SetShading(bool smooth)
	{
		if (smooth != m_SmoothShading)
		{
			m_SmoothShading = smooth;
			Invalidate();
		}
	}
}

