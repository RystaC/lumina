#include <chrono>
#include <fstream>
#include <format>
#include <iostream>
#include <random>
#include <semaphore>
#include <thread>

#include <cstdint>

#include "lumina/lumina.hpp"

#if defined(NDEBUG)
#define BUILD_TYPE "Release"
#if defined(DEBUG)
#undef DEBUG
#elif defined(_DEBUG)
#undef _DEBUG
#endif
#else
#define BUILD_TYPE "Debug"
#if !defined(DEBUG)
#define DEBUG
#endif
#endif

constexpr lumina::f32 ASPECT_RATIO = 16.0f / 9.0f;
constexpr lumina::u32 IMAGE_WIDTH  = 512;
constexpr lumina::u32 IMAGE_HEIGHT = (IMAGE_WIDTH / ASPECT_RATIO < 1) ? 1 : IMAGE_WIDTH / ASPECT_RATIO;
#if defined(DEBUG)
constexpr lumina::u32 SAMPLES      = 1;
constexpr lumina::u32 MAX_RECURSE  = 4;
#else
constexpr lumina::u32 SAMPLES      = 128;
constexpr lumina::u32 MAX_RECURSE  = 32;
#endif

template<class RandGen>
lumina::vec3f32 trace_ray(const lumina::ray& ray, const lumina::bvh& bvh, const std::vector<lumina::vec3f32>& vertices, const std::vector<lumina::vec3u32>& indices, const std::vector<lumina::u32>& mat_indices, const std::vector<lumina::material>& materials, RandGen& rng) {
    lumina::ray current_ray = ray;
    lumina::vec3f32 current_att = lumina::vec3f32(1.0f);

    for(auto i = 0; i < MAX_RECURSE; ++i) {
        lumina::f32 t_max = lumina::F32_MAX;
        auto test_res = bvh.trace(vertices, indices, current_ray, lumina::F32_MAX);

        if(test_res) {
            auto index_idx = test_res->first;
            const auto& mat = materials[mat_indices[index_idx]];
            auto t = test_res->second;
            lumina::triangle triangle(vertices[indices[index_idx].x], vertices[indices[index_idx].y], vertices[indices[index_idx].z]);

            current_att *= mat.albedo + mat.emission;

            auto n = triangle.normal(ray, t);
            auto next_origin = current_ray[t] + n * 0.0001f;
            // auto next_direction = lumina::sample_cosine_hemisphere(n, rng);
            auto [next_direction, bsdf_value, pdf_value] = lumina::sample_bsdf(current_ray.direction, n, mat.roughness, 1.0f, mat.refractive_index, rng);
            current_ray = lumina::ray(next_origin, next_direction);
        }
        else {   
            auto t = 0.5f * (ray.direction.y + 1.0f);
            auto c = (1.0f - t) * lumina::vec3f32(1.0f) + t * lumina::vec3f32(0.5f, 0.7f, 1.0f);
            return current_att * c;
        }
    }

    return lumina::vec3f32(0.0f);
}

void save_ppm(const std::filesystem::path& path, const std::vector<lumina::vec3f32>& pixels) {
    std::ofstream ofs(path);
    if(ofs.fail()) {
        std::clog << std::format("failed to create file: {}. exit.", "test.ppm") << std::endl;
        std::exit(EXIT_FAILURE);
    }

    ofs << std::format("P3\n{} {}\n255\n", IMAGE_WIDTH, IMAGE_HEIGHT);

    for(const auto& p : pixels) {
        ofs << std::format("{} {} {}\n", lumina::u8(p.r * 255.999f), lumina::u8(p.g * 255.999f), lumina::u8(p.b * 255.999f));
    }

    ofs.close();
}

int main(int argc, const char* argv[]) {
    std::cout << std::format("build type: {}", BUILD_TYPE) << std::endl;

    std::random_device seed{};
    lumina::xoshiro256pp rng0(seed());

    lumina::camera cam(
        {0.0f, 1.0f, -1.0f},
        {0.0f, 0.7f, -0.5f},
        {0.0f, 1.0f, 0.0f},
        90.0f, IMAGE_WIDTH, IMAGE_HEIGHT
    );

    // auto first_ray = cam.generate_ray(0, 0);
    // auto n = lumina::vec3f32(0.0f, 1.0f, 0.0f);
    // auto roughness = 0.5f;
    // auto n1 = 1.0f;
    // auto n2 = 1.5f;
    // std::cout << std::format("first ray = {}", first_ray) << std::endl;
    // auto [second_ray, bsdf_value, pdf_value] = lumina::sample_bsdf(first_ray.direction, -first_ray.direction, roughness, n1, n2, rng0);
    // std::cout << std::format("second ray dir = {}, BSDF value = {}, PDF value = {}", second_ray, bsdf_value, pdf_value) << std::endl;

    // return 0;

    auto [vertices, indices, mesh_groups] = lumina::load_obj("../asset/mori_knob/mori_knob.obj");
    std::vector<lumina::u32> mat_indices(indices.size());
    std::vector<lumina::material> materials(mesh_groups.size());

    std::unordered_map<std::string, lumina::material> mat_config{};
    mat_config["BackGroundMat"] = lumina::material{.albedo = {0.8f}, .emission = {0.0f}, .roughness = 0.05f, .refractive_index = 0.0f};
    mat_config["InnerMat"] = lumina::material{.albedo = {0.8f, 0.8f, 0.0f}, .emission = {0.0f}, .roughness = 1.0f, .refractive_index = 0.0f};
    mat_config["LTELogo"] = lumina::material{.albedo = {0.0f, 0.8f, 0.0f}, .emission = {0.8f}, .roughness = 1.0f, .refractive_index = 0.0f};
    mat_config["Material"] = lumina::material{.albedo = {1.0f}, .emission = {0.0f}, .roughness = 1.0f, .refractive_index = 0.0f};
    mat_config["OuterMat"] = lumina::material{.albedo = {0.8f, 0.0f, 0.0f}, .emission = {0.0f}, .roughness = 0.5f, .refractive_index = 0.0f};

    {
        lumina::u32 m = 0;
        std::uniform_real_distribution<lumina::f32> rand_color{};

        for(const auto& [name, group] : mesh_groups) {
            std::cout << std::format("{}, start = {}, end = {}", name, group.first, group.second) << std::endl;
            for(auto i = group.first; i < group.second; ++i) {
                mat_indices[i] = m;
            }
            materials[m] = mat_config[name];
            ++m;
        }
    }

    std::cout << std::format("possible # of threads = {}", std::thread::hardware_concurrency()) << std::endl;

    lumina::bvh bvh(vertices, indices);

    auto time_start = std::chrono::steady_clock::now();

    std::vector<lumina::vec3f32> pixels(IMAGE_WIDTH * IMAGE_HEIGHT, 0.0f);

    auto thread_count = std::max<lumina::u32>(1, std::thread::hardware_concurrency());
    std::vector<std::thread> threads{};
 
    for(auto i = 0; i < thread_count; ++i) {
        auto split_width = IMAGE_WIDTH / thread_count;
        threads.push_back(
            std::thread([&](lumina::u32 tid, std::random_device::result_type seed) {
                lumina::xoshiro256pp rng(seed);

                for(lumina::u32 y = 0; y < IMAGE_HEIGHT; ++y) {
                    std::clog << std::format("\rprogress: thread {:>2} > {:>3.1f}% ({:>4}/{:>4})", tid, (float(y) / (IMAGE_HEIGHT - 1)) * 100, y + 1, IMAGE_HEIGHT) << std::flush;
                    for(lumina::u32 x = tid * split_width; x < std::min((tid + 1) * split_width, IMAGE_WIDTH); ++x) {
                        lumina::vec3f32 pixel{};
                        for(lumina::u32 s = 0; s < SAMPLES; ++s) {
                            auto ray = cam.generate_ray(x, y, rng);

                            pixel += trace_ray(ray, bvh, vertices, indices, mat_indices, materials, rng);
                        }
                        pixel /= lumina::f32(SAMPLES);
                        pixels[y * IMAGE_WIDTH + x] = pixel;
                    }
                }
            }, i, seed())
        );
    }

    for(auto& t : threads) {
        t.join();
    }

    save_ppm("test.ppm", pixels);

    auto time_end = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start).count();

    std::clog << std::format("\nfinished. elapsed time: {} sec", elapsed) << std::endl;

    return 0;
}