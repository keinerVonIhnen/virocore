//
//  VROAudioPlayeriOS.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/6/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROAudioPlayeriOS.h"
#include "VROData.h"
#include "VROLog.h"
#include "VROPlatformUtil.h"
#include "VROMath.h"

VROAudioPlayeriOS::VROAudioPlayeriOS(std::string url, bool isLocalUrl) :
    _playVolume(1.0),
    _muted(false),
    _paused(false),
    _loop(false) {
        
    if (isLocalUrl) {
        _player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]]
                                                     error:NULL];
        [_player prepareToPlay];
        _delegate->soundIsReady();
    }
    else {
        // download to file
        NSURL *urlObj = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
        downloadDataWithURL(urlObj, ^(NSData *data, NSError *error) {
            _player = [[AVAudioPlayer alloc] initWithData:data error:NULL];
            updatePlayerProperties();
            
            _audioDelegate = [[VROAudioPlayerDelegate alloc] initWithSoundDelegate:_delegate];
            _player.delegate = _audioDelegate;
            _delegate->soundIsReady();
        });
    }
}

VROAudioPlayeriOS::VROAudioPlayeriOS(std::shared_ptr<VROData> data) :
    _playVolume(1.0),
    _muted(false),
    _paused(false),
    _loop(false) {
    
    _player = [[AVAudioPlayer alloc] initWithData:[NSData dataWithBytes:data->getData() length:data->getDataLength()]
                                            error:NULL];
    [_player prepareToPlay];
}

std::shared_ptr<VROAudioPlayeriOS> VROAudioPlayeriOS::create(std::shared_ptr<VROSoundData> data) {
    std::shared_ptr<VROAudioPlayeriOS> player = std::make_shared<VROAudioPlayeriOS>(data);
    player->setup();
    return player;
}

VROAudioPlayeriOS::VROAudioPlayeriOS(std::shared_ptr<VROSoundData> data) :
    _playVolume(1.0),
    _muted(false),
    _paused(false),
    _loop(false) {
        
    _data = data;
}

void VROAudioPlayeriOS::setup() {
    if (_data) {
        _data->setDelegate(shared_from_this());
    }
}

VROAudioPlayeriOS::~VROAudioPlayeriOS() {
    if (_player) {
        [_player stop];
    }
}

void VROAudioPlayeriOS::updatePlayerProperties() {
    _player.numberOfLoops = _loop ? -1 : 0;
    _player.volume = _muted ? 0 : _playVolume;
}

void VROAudioPlayeriOS::setLoop(bool loop) {
    if (_loop == loop) {
        return;
    }
    
    _loop = loop;
    if (_player) {
        _player.numberOfLoops = loop ? -1 : 0;
        
        // If we were not explicitly paused and loop was activated,
        // play the sound (so it turns back on)
        if (!_paused && _loop) {
            [_player play];
        }
    }
}

void VROAudioPlayeriOS::setMuted(bool muted) {
    _muted = muted;
    if (_player) {
        _player.volume = muted ? 0 : _playVolume;
    }
}

void VROAudioPlayeriOS::seekToTime(float seconds) {
    if (_player) {
        seconds = clamp(seconds, 0, _player.duration);
        _player.currentTime = seconds;
    }
}

void VROAudioPlayeriOS::setVolume(float volume) {
    _playVolume = volume;
    if (_player) {
        _player.volume = volume;
    }
}

void VROAudioPlayeriOS::play() {
    _paused = false;
    if (_player) {
        _player.volume = _playVolume;
        [_player play];
    }
}

void VROAudioPlayeriOS::pause() {
    _paused = true;
    if (_player) {
        doFadeThenPause();
    }
}

void VROAudioPlayeriOS::doFadeThenPause() {
    // Grab the shared_ptr to this, so that we retain this in the dispatch_after;
    // otherwise only 'this' is captured, so the ref-count can slip to zero and
    // cause a crash
    std::shared_ptr<VROAudioPlayeriOS> capturedSelf = shared_from_this();
    
    if (_player.volume > 0.1) {
        _player.volume = _player.volume - 0.1;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            capturedSelf->doFadeThenPause();
        });
    }
    else {
        [_player pause];
    }
}

void VROAudioPlayeriOS::dataIsReady() {
    if (!_player) {
        _player = [[AVAudioPlayer alloc] initWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String:_data->getLocalFilePath().c_str()]]
                                                         error:NULL];
        [_player prepareToPlay];
        updatePlayerProperties();
        
        if (!_paused) {
            [_player play];
        }
    }
}

#pragma mark - VROAudioPlayerDelegate implementation
@implementation VROAudioPlayerDelegate {
    std::weak_ptr<VROSoundDelegateInternal> _delegate;
}

- (id)initWithSoundDelegate:(std::shared_ptr<VROSoundDelegateInternal>)soundDelegate {
    self = [super init];
    if (self) {
        _delegate = soundDelegate;
    }
    return self;
}

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag {
    std::shared_ptr<VROSoundDelegateInternal> soundDelegate = _delegate.lock();
    if (soundDelegate) {
        soundDelegate->soundDidFinish();
    }
}

@end
