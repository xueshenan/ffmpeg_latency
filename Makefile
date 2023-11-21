CC=/opt/android/android-ndk-r26b/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang
CXX=/opt/android/android-ndk-r26b/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang++

ffmpeg_latency: main.cc
	$(CXX) main.cc -o ffmpeg_latency \
		-I ./third_party/release/include -L./third_party/release/lib \
		-lavfilter -lavformat -lavcodec -lavutil -lz
