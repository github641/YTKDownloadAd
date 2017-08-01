//
//  YTKChainRequest.m
//
//  Copyright (c) 2012-2016 YTKNetwork https://github.com/yuantiku
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#import "YTKChainRequest.h"
#import "YTKChainRequestAgent.h"
#import "YTKNetworkPrivate.h"
#import "YTKBaseRequest.h"

@interface YTKChainRequest()<YTKRequestDelegate>

@property (strong, nonatomic) NSMutableArray<YTKBaseRequest *> *requestArray;
@property (strong, nonatomic) NSMutableArray<YTKChainCallback> *requestCallbackArray;
@property (assign, nonatomic) NSUInteger nextRequestIndex;
@property (strong, nonatomic) YTKChainCallback emptyCallback;

@end

@implementation YTKChainRequest

/* lzy注170718：
 一般初始化方法，alloc init。
 
 这个类没有使用：
 - (instancetype)init NS_UNAVAILABLE;
 + (instancetype)new NS_UNAVAILABLE;
 
 初始化工作如下
 1、初始『下一个请求的索引』为0
 2、初始化存放所有请求的数组
 3、初始化存放请求的回调的数组
 4、初始化一个链式请求完成block
 
 */
- (instancetype)init {
    self = [super init];
    if (self) {
        _nextRequestIndex = 0;
        _requestArray = [NSMutableArray array];
        _requestCallbackArray = [NSMutableArray array];
        _emptyCallback = ^(YTKChainRequest *chainRequest, YTKBaseRequest *baseRequest) {
            // do nothing
        };
    }
    return self;
}
/* lzy注170718：
 通过把链式请求数组中的第一个请求添加到请求队列中，来开始这个链式请求。
 1、判断『下一个请求的索引』若大于0，YTKLog打印错误，即链式请求已经开始了，直接return。
 2、判断『管理链式请求中所有请求』的数组是否为空：
     1）、为空，使用YTKLog打印其请求为空的日志
     2）、不为空，next
 3、通知『请求配件』这个时间点；调用- (BOOL)startNextRequest这个方法；调用『链式请求中介实例』的addChainRequest方法，添加该链式请求。
 */
- (void)start {
    if (_nextRequestIndex > 0) {
        YTKLog(@"Error! Chain request has already started.");
        return;
    }

    if ([_requestArray count] > 0) {
        [self toggleAccessoriesWillStartCallBack];
        [self startNextRequest];
        [[YTKChainRequestAgent sharedAgent] addChainRequest:self];
    } else {
        YTKLog(@"Error! Chain request array is empty.");
    }
}
/* lzy注170718：
 停止这个链式请求。链式请求中剩下的请求将会被cancelled。
 1、通知『请求配件』这个时间点
 2、清理子请求
 3、『链式请求中介实例』将本类实例移除
 */
- (void)stop {
    [self toggleAccessoriesWillStopCallBack];
    [self clearRequest];
    [[YTKChainRequestAgent sharedAgent] removeChainRequest:self];
    [self toggleAccessoriesDidStopCallBack];
}
/* lzy注170718：
 给链式请求对象，添加一个带完成回调的请求。
 1、请求放到数组中管理。
 2、传入了完成回调，那么该回调放到『管理链式请求回调的数组』中
 3、没有传入完成回调，使用本类实例init时，初始化的空的链式请求完成回调。
 */
- (void)addRequest:(YTKBaseRequest *)request callback:(YTKChainCallback)callback {
    [_requestArray addObject:request];
    if (callback != nil) {
        [_requestCallbackArray addObject:callback];
    } else {
        [_requestCallbackArray addObject:_emptyCallback];
    }
}

- (NSArray<YTKBaseRequest *> *)requestArray {
    return _requestArray;
}
/* lzy注170718：
 链式请求，开启下一个请求。
 1、『下一个请求索引』在链式请求『所有子请求数组』是否越界，
 越界直接返回『开启失败』NO，
 未越界，next
 2、从链式请求『所有子请求数组』中，根据『下一个请求索引』，取出该子请求；『下一个请求索引+1』
 3、子请求delegate对象设置为本类实例，清理子请求的block回调形式，统一使用delegate做回调。
 #突然想到，block形式无法很好的组织和处理多个请求，使用delegate，所有的子请求都将在一个地方回调，便于逻辑的处理。#
 4、子请求start
 */
- (BOOL)startNextRequest {
    if (_nextRequestIndex < [_requestArray count]) {
        YTKBaseRequest *request = _requestArray[_nextRequestIndex];
        _nextRequestIndex++;
        request.delegate = self;
        [request clearCompletionBlock];
        [request start];
        return YES;
    } else {
        return NO;
    }
}

#pragma mark - Network Request Delegate
/* lzy注170718：
 子请求的delegate对象设置为本类实例，则所有子请求的成功失败时间点都将回调到这里。
 */

/* lzy注170718：
 子请求成功完成的回调。
 1、取出当前的子请求：当前子请求的索引 = 下一个请求的索引 - 1
 2、取出当前子请求添加的链式请求callback，callback这个时间点
 3、尝试开始下一个请求，
      开启下一个请求若失败，
     1）通知『请求配件』该时间点
     2）若本类实例的delegate有值且实现了『链式请求完成』的方法，通知该方法，紧接着在『链式请求中介』中移除这个链式请求。
 */
- (void)requestFinished:(YTKBaseRequest *)request {
    NSUInteger currentRequestIndex = _nextRequestIndex - 1;
    YTKChainCallback callback = _requestCallbackArray[currentRequestIndex];
    callback(self, request);
    if (![self startNextRequest]) {
        [self toggleAccessoriesWillStopCallBack];
        if ([_delegate respondsToSelector:@selector(chainRequestFinished:)]) {
            [_delegate chainRequestFinished:self];
            [[YTKChainRequestAgent sharedAgent] removeChainRequest:self];
        }
        [self toggleAccessoriesDidStopCallBack];
    }
}

/* lzy注170718：
 子请求失败的delegate方法。
 1、通知『请求配件』该时间点。
 2、若本类实例delegate对象有值，且实现了『链式请求失败』方法，通知该方法，紧接着在『链式请求中介』中移除这个链式请求。
 */
- (void)requestFailed:(YTKBaseRequest *)request {
    [self toggleAccessoriesWillStopCallBack];
    if ([_delegate respondsToSelector:@selector(chainRequestFailed:failedBaseRequest:)]) {
        [_delegate chainRequestFailed:self failedBaseRequest:request];
        [[YTKChainRequestAgent sharedAgent] removeChainRequest:self];
    }
    [self toggleAccessoriesDidStopCallBack];
}
/* lzy注170718：
 清理子请求。
 1、取到当前请求的索引：下一个请求的索引-1
 2、若 当前请求的索引 在 所有子请求数组 中未越界，取出该子请求，并调用子请求的stop方法
 3、所有子请求数组 移除所有的元素
 4、链式请求回调数组，移除所有的元素
 */
- (void)clearRequest {
    NSUInteger currentRequestIndex = _nextRequestIndex - 1;
    if (currentRequestIndex < [_requestArray count]) {
        YTKBaseRequest *request = _requestArray[currentRequestIndex];
        [request stop];
    }
    [_requestArray removeAllObjects];
    [_requestCallbackArray removeAllObjects];
}

#pragma mark - Request Accessoies

- (void)addAccessory:(id<YTKRequestAccessory>)accessory {
    if (!self.requestAccessories) {
        self.requestAccessories = [NSMutableArray array];
    }
    [self.requestAccessories addObject:accessory];
}

@end
