#include <chrono>
#include <fstream>
#include <format>
#include <iostream>
#include <random>
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
lumina::vec3f32 trace_ray(const lumina::ray& ray, const lumina::bvh& bvh, const std::vector<lumina::vec3f32>& vertices, const std::vector<lumina::vec3u32>& indices, RandGen& rng) {
    lumina::ray current_ray = ray;
    lumina::f32 current_att = 1.0f;

    for(auto i = 0; i < MAX_RECURSE; ++i) {
        lumina::f32 t_max = lumina::F32_MAX;
        auto test_res = bvh.trace(vertices, indices, current_ray, lumina::F32_MAX);

        if(test_res) {
            auto index_idx = test_res->first;
            auto t = test_res->second;
            lumina::triangle triangle(vertices[indices[index_idx].x], vertices[indices[index_idx].y], vertices[indices[index_idx].z]);

            current_att *= 0.5f;

            auto n = triangle.normal(ray, t);
            current_ray = lumina::ray(current_ray[t] + n * 0.0001f, lumina::sample_hemisphere(n, rng));
        }
        else {   
            auto t = 0.5f * (ray.direction.y + 1.0f);
            auto c = (1.0f - t) * lumina::vec3f32(1.0f) + t * lumina::vec3f32(0.5f, 0.7f, 1.0f);
            return current_att * c;
        }
    }

    return lumina::vec3f32(0.0f);
}

void write_pixels(std::ostream& os, const std::vector<lumina::vec3f32>& pixels) {
    for(const auto& p : pixels) {
        os << std::format("{} {} {}\n", lumina::u8(p.r * 255.999f), lumina::u8(p.g * 255.999f), lumina::u8(p.b * 255.999f));
    }
}

int main(int argc, const char* argv[]) {
    std::cout << std::format("build type: {}", BUILD_TYPE) << std::endl;

    std::random_device seed{};

    lumina::camera cam(
        {0.0f, 1.0f, -1.0f},
        {0.0f, 0.7f, -0.5f},
        {0.0f, 1.0f, 0.0f},
        90.0f, IMAGE_WIDTH, IMAGE_HEIGHT
    );

    auto [vertices, indices] = lumina::load_obj("../asset/mori_knob/mori_knob.obj");
    std::cout << std::format("# of vertices = {}, # of faces = {}", vertices.size(), indices.size()) << std::endl;

    std::cout << std::format("possible # of threads = {}", std::thread::hardware_concurrency()) << std::endl;

    lumina::bvh bvh(vertices, indices);

    std::ofstream ofs("test.ppm");
    if(ofs.fail()) {
        std::clog << std::format("failed to create file: {}. exit.", "test.ppm") << std::endl;
        std::exit(EXIT_FAILURE);
    }

    ofs << std::format("P3\n{} {}\n255\n", IMAGE_WIDTH, IMAGE_HEIGHT);

    auto time_start = std::chrono::steady_clock::now();

    std::vector<lumina::vec3f32> pixels(IMAGE_WIDTH * IMAGE_HEIGHT);

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

                            pixel += trace_ray(ray, bvh, vertices, indices, rng);
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

    write_pixels(ofs, pixels);

    auto time_end = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(time_end - time_start).count();

    std::clog << std::format("\nfinished. elapsed time: {} sec", elapsed) << std::endl;

    ofs.close();

    return 0;
}