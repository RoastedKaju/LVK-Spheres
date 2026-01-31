#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_set>

namespace fs = std::filesystem;

inline std::string readTextFile(const fs::path& file, std::unordered_set<fs::path>& includeGuard)
{
	if (!includeGuard.insert(fs::absolute(file)).second)
	{
		LLOGW("Circular include detected: %s\n", file.string().c_str());
		return {};
	}

	std::ifstream in(file, std::ios::binary);
	if (!in)
	{
		LLOGW("Failed to open text file %s\n", file.string().c_str());
		return {};
	}

	std::string code;
	in.seekg(0, std::ios::end);
	code.reserve(in.tellg());
	in.seekg(0, std::ios::beg);
	code.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

	// Remove UTF-8 BOM
	if (code.size() >= 3 &&
		static_cast<unsigned char>(code[0]) == 0xEF &&
		static_cast<unsigned char>(code[1]) == 0xBB &&
		static_cast<unsigned char>(code[2]) == 0xBF)
	{
		code.erase(0, 3);
	}

	// Handle #include <file>
	size_t pos = 0;
	while ((pos = code.find("#include", pos)) != std::string::npos)
	{
		const auto start = code.find('<', pos);
		const auto end = code.find('>', start);

		if (start == std::string::npos || end == std::string::npos)
			break;

		const fs::path includePath = file.parent_path() / code.substr(start + 1, end - start - 1);

		const std::string includeCode = readTextFile(includePath, includeGuard);

		code.replace(pos, end - pos + 1, includeCode);
	}

	return code;
}

inline std::string readShaderFile(const fs::path& file)
{
	std::unordered_set<fs::path> guard;
	return readTextFile(file, guard);
}

inline lvk::ShaderStage shaderStageFromPath(const fs::path& file)
{
	const auto extension = file.extension().string();

	if (extension == ".vert") return lvk::Stage_Vert;
	if (extension == ".frag") return lvk::Stage_Frag;
	if (extension == ".geom") return lvk::Stage_Geom;
	if (extension == ".comp") return lvk::Stage_Comp;
	if (extension == ".tesc") return lvk::Stage_Tesc;
	if (extension == ".tese") return lvk::Stage_Tese;

	LLOGW("Unknown shader extension: %s\n", extension.c_str());
	return lvk::Stage_Vert;
}

inline lvk::Holder<lvk::ShaderModuleHandle> loadShaderModule(const std::unique_ptr<lvk::IContext>& ctx, const std::filesystem::path& file)
{
	const std::string code = readShaderFile(file);
	const lvk::ShaderStage stage = shaderStageFromPath(file);

	if (code.empty())
		return {};

	lvk::Result result;

	lvk::Holder<lvk::ShaderModuleHandle> handle = ctx->createShaderModule({ code.c_str(), stage, (std::string("Shader module : ") + file.string()).c_str() }, &result);

	if (!result.isOk())
		return {};

	LLOGL("Loaded shader module from file: %s\n", file.string().c_str());
	return handle;
}