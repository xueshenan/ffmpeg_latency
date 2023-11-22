# CC=/opt/android/android-ndk-r26b/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang
# CXX=/opt/android/android-ndk-r26b/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android30-clang++
CC=/Users/tbago/Library/Android/android-ndk-r26b/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android30-clang
CXX=/Users/tbago/Library/Android/android-ndk-r26b/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android30-clang++

ffmpeg_latency: main.cc
	$(CXX) main.cc -o ffmpeg_latency \
		-I ./third_party/release/include -L./third_party/release/lib \
		-lavfilter -lavformat -lavcodec -lavutil -lz
