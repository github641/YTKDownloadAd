//
//  YTKBatchRequest.m
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

#import "YTKBatchRequest.h"
#import "YTKNetworkPrivate.h"
#import "YTKBatchRequestAgent.h"
#import "YTKRequest.h"

@interface YTKBatchRequest() <YTKRequestDelegate>

/* lzy注170718：
 内部属性，批量请求的完成个数。
 */
@property (nonatomic) NSInteger finishedCount;

@end

@implementation YTKBatchRequest
/* lzy注170718：
 传入一组请求来创建`YTKBatchRequest`批量请求。
 1、持有住 请求数组
 2、初始化 请求完成个数为0
 3、遍历请求数组，检查每一个对象是否是YTKRequest对象，
     类型检查通过，初始化成功；
     类型检查出错，YTKLog打印，初始化失败返回nil。
 */
- (instancetype)initWithRequestArray:(NSArray<YTKRequest *> *)requestArray {
    self = [super init];
    if (self) {
        /* lzy注170718：
         使用了数组的copy方法，记不清楚看这里：小结iOS中的copyhttp://www.jianshu.com/p/5254f1277dba
         */
        _requestArray = [requestArray copy];
        _finishedCount = 0;
        
        for (YTKRequest * req in _requestArray) {
            if (![req isKindOfClass:[YTKRequest class]]) {
                YTKLog(@"Error, request item must be YTKRequest instance.");
                return nil;
            }
        }
    }
    return self;
}

    /* TODO: #待完成# 
     （之前调用了start方法，但是_finishedCount还是0如何解决？？？）
     */

/* lzy注170718：
 将所有的请求添加到队列中。
 1、检查_finishedCount是否大于0，大于的话YTKLog打印『批量请求已经开始』，并直接return。
 2、初始化一个YTKRequest作为第一个请求失败的Request的容器
 3、是用本类实例，初始化『批量请求中介YTKBatchRequestAgent』
 4、遍历请求数组，取出数组中req启动请求，并把本类设置为req的delegate
 */
- (void)start {
    if (_finishedCount > 0) {
        YTKLog(@"Error! Batch request has already started.");
        return;
    }
    _failedRequest = nil;
    [[YTKBatchRequestAgent sharedAgent] addBatchRequest:self];
    [self toggleAccessoriesWillStartCallBack];
    for (YTKRequest * req in _requestArray) {
        req.delegate = self;
        [req clearCompletionBlock];
        [req start];
    }
}
/* lzy注170718：
 停止『批量请求』中的所有请求。
 1、回调『请求配件』这个时间点
 2、本类实例delegate置空
 3、请求数组中请求挨个取消
 4、从『批量请求中介batchRequestAgent』中移除本类实例
 */
- (void)stop {
    [self toggleAccessoriesWillStopCallBack];
    _delegate = nil;
    [self clearRequest];
    [self toggleAccessoriesDidStopCallBack];
    [[YTKBatchRequestAgent sharedAgent] removeBatchRequest:self];
}
/* lzy注170718：
 方便地启动一个带回调的『批量请求』
 1、内部持有住成功失败block
 2、调用本类的start方法。
 */
- (void)startWithCompletionBlockWithSuccess:(void (^)(YTKBatchRequest *batchRequest))success
                                    failure:(void (^)(YTKBatchRequest *batchRequest))failure {
    [self setCompletionBlockWithSuccess:success failure:failure];
    [self start];
}
/* lzy注170718：
 设置批量请求的完成回调，包括成功、失败回调。
 */
- (void)setCompletionBlockWithSuccess:(void (^)(YTKBatchRequest *batchRequest))success
                              failure:(void (^)(YTKBatchRequest *batchRequest))failure {
    self.successCompletionBlock = success;
    self.failureCompletionBlock = failure;
}
/* lzy注170718：
 把成功、失败回调置空。
 */
- (void)clearCompletionBlock {
    // nil out to break the retain cycle.
    self.successCompletionBlock = nil;
    self.failureCompletionBlock = nil;
}
/* lzy注170718：
 是否所有请求的返回数据都是从本地缓存中来的。
 */
- (BOOL)isDataFromCache {
    BOOL result = YES;
    for (YTKRequest *request in _requestArray) {
        if (!request.isDataFromCache) {
            result = NO;
        }
    }
    return result;
}


- (void)dealloc {
    [self clearRequest];
}

#pragma mark - Network Request Delegate

/* lzy注170718：
 1、请求完成数 + 1
 2、经过上一步后，若请求完成书 等于 批量请求数组中请求数，则所有请求都完成了。
   1）、回调相关各方时间点（请求配件、本类delegate、回调block，回调完后置空）
   2）、从『批量请求中介YTKBatchRequestAgent』中移除本类实例
 */
- (void)requestFinished:(YTKRequest *)request {
    _finishedCount++;
    if (_finishedCount == _requestArray.count) {
        [self toggleAccessoriesWillStopCallBack];
        if ([_delegate respondsToSelector:@selector(batchRequestFinished:)]) {
            [_delegate batchRequestFinished:self];
        }
        if (_successCompletionBlock) {
            _successCompletionBlock(self);
        }
        [self clearCompletionBlock];
        [self toggleAccessoriesDidStopCallBack];
        [[YTKBatchRequestAgent sharedAgent] removeBatchRequest:self];
    }
}
/* lzy注170718：
 1、给『请求失败的Request的容器』赋值请求失败的该请求对象
 2、给 『请求配件』回调
 3、遍历请求数组，挨个调用req的stop方法
 4、若有delegate，回调请求失败的delegate方法
 5、若有block，回调请求失败block，之后置空所有请求回调block
 6、从『批量请求中介YTKBatchRequestAgent』中移除本类实例
 */
- (void)requestFailed:(YTKRequest *)request {
    _failedRequest = request;
    [self toggleAccessoriesWillStopCallBack];
    // Stop
    for (YTKRequest *req in _requestArray) {
        [req stop];
    }
    // Callback
    if ([_delegate respondsToSelector:@selector(batchRequestFailed:)]) {
        [_delegate batchRequestFailed:self];
    }
    if (_failureCompletionBlock) {
        _failureCompletionBlock(self);
    }
    // Clear
    [self clearCompletionBlock];

    [self toggleAccessoriesDidStopCallBack];
    [[YTKBatchRequestAgent sharedAgent] removeBatchRequest:self];
}
/* lzy注170718：
 清理请求。
 1、停止『批量请求』中的所有请求。
 2、置空所有回调block
 */
- (void)clearRequest {
    for (YTKRequest * req in _requestArray) {
        [req stop];
    }
    [self clearCompletionBlock];
}

#pragma mark - Request Accessoies

- (void)addAccessory:(id<YTKRequestAccessory>)accessory {
    if (!self.requestAccessories) {
        self.requestAccessories = [NSMutableArray array];
    }
    [self.requestAccessories addObject:accessory];
}

@end
