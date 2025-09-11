#include <chrono>
#include <fstream>
#include <format>
#include <iostream>
#include <mutex>
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
constexpr lumina::u32 SAMPLES      = 1024;
constexpr lumina::u32 MAX_RECURSE  = 32;
#endif

template<class RandGen>
lumina::vec3f32 trace_ray(const lumina::ray& ray, const lumina::bvh& bvh, const lumina::mesh& mesh, RandGen& rng) {
    lumina::ray current_ray = ray;
    lumina::vec3f32 current_att = lumina::vec3f32(1.0f);

    for(auto i = 0; i < MAX_RECURSE; ++i) {
        lumina::f32 t_max = lumina::F32_MAX;
        auto test_result = bvh.trace(mesh.vertices, mesh.vertex_indices, current_ray, lumina::F32_MAX);

        if(test_result) {
            auto index_index = test_result->first;
            const auto& material = mesh.material(index_index);
            auto t = test_result->second;

            current_att *= material.albedo + material.emission;
            if(lumina::dot(material.emission, material.emission) > 0.0f) {
                return current_att;
            }

            auto n = mesh.normal(ray[t], index_index);
            n = dot(ray.direction, n) > 0.0f ? -n : n;

            auto next_origin = current_ray[t] + n * 0.0001f;
            auto [next_direction, bsdf_value, pdf_value] = lumina::sample_bsdf(current_ray.direction, n, material.roughness, 1.0f, material.refractive_index, rng);
            current_ray = lumina::ray(next_origin, next_direction);
        }
        else {   
            auto background = lumina::vec3f32(0.0f);
            return background;
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
        {1.0f, 1.0f, -1.0f},
        {0.0f, 0.7f, -0.5f},
        {0.0f, 1.0f, 0.0f},
        90.0f, IMAGE_WIDTH, IMAGE_HEIGHT
    );

    auto [vertices, texcoords, normals, vertex_indices, texcoord_indices, normal_indices, mesh_groups] = lumina::load_obj("../asset/mori_knob/mori_knob.obj");
    lumina::mesh mesh(std::move(vertices), std::move(texcoords), std::move(normals), std::move(vertex_indices), std::move(texcoord_indices), std::move(normal_indices), std::move(mesh_groups));
    mesh.add_material("BackGroundMat", lumina::material{.albedo = {0.8f}, .emission = {0.0f}, .roughness = 0.2f, .refractive_index = 0.0f});
    mesh.add_material("InnerMat", lumina::material{.albedo = {0.8f, 0.8f, 0.0f}, .emission = {0.0f}, .roughness = 1.0f, .refractive_index = 0.0f});
    mesh.add_material("LTELogo", lumina::material{.albedo = {0.0f, 0.8f, 0.0f}, .emission = {0.0f, 0.8f, 0.0f}, .roughness = 1.0f, .refractive_index = 0.0f});
    mesh.add_material("Material", lumina::material{.albedo = {1.0f}, .emission = {1.0f}, .roughness = 1.0f, .refractive_index = 0.0f});
    mesh.add_material("OuterMat", lumina::material{.albedo = {0.8f, 0.0f, 0.0f}, .emission = {0.0f}, .roughness = 0.0f, .refractive_index = 0.0f});
    mesh.statistics();

    std::cout << std::format("possible # of threads = {}", std::thread::hardware_concurrency()) << std::endl;

    lumina::bvh bvh(mesh.vertices, mesh.vertex_indices);

    auto time_start = std::chrono::steady_clock::now();

    std::vector<lumina::vec3f32> pixels(IMAGE_WIDTH * IMAGE_HEIGHT, 0.0f);

    auto thread_count = std::max<lumina::u32>(1, std::thread::hardware_concurrency());
    // auto thread_count = 1;
    std::vector<std::thread> threads{};
    std::queue<std::pair<lumina::u32, lumina::u32>> task_queue{};
    for(auto y = 0; y < IMAGE_HEIGHT; ++y) {
        for(auto x = 0; x < IMAGE_WIDTH; ++x) {
            task_queue.push(std::make_pair(x, y));
        }
    }

    auto total_pixels = task_queue.size();

    std::mutex queue_lock{};

    for(auto i = 0; i < thread_count; ++i) {
        threads.push_back(
            std::thread(
                [&](std::random_device::result_type seed) {
                    lumina::xoshiro256pp rng(seed);

                    while(true) {
                        queue_lock.lock();
                        if(task_queue.empty()) {
                            queue_lock.unlock();
                            break;
                        }
                        auto [x, y] = task_queue.front();
                        task_queue.pop();
                        std::clog << std::format("\rprogress: {:.2f}% ({:>6}/{:>6})", lumina::f32(total_pixels - task_queue.size()) / lumina::f32(total_pixels) * 100.0f, total_pixels - task_queue.size(), total_pixels) << std::flush;
                        queue_lock.unlock();
                        
                        lumina::vec3f32 pixel{};
                        for(lumina::u32 s = 0; s < SAMPLES; ++s) {
                            auto ray = cam.generate_ray(x, y, rng);

                            pixel += lumina::min(trace_ray(ray, bvh, mesh, rng), lumina::vec3f32(1.0f));
                        }
                        pixel /= lumina::f32(SAMPLES);
                        pixels[y * IMAGE_WIDTH + x] = pixel;
                    }
                },
                seed()
            )
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