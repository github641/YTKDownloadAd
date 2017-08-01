//
//  YTKChainRequest.h
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
/* lzy注170713：
 链式请求，也就是请求之间互相依赖，串行发出。
 和批量请求相似的，通过一个数组来管理这些依赖的请求。
 内部通过_nextRequestIndex来索引正在进行和下一个将要处理的请求，每次上一个请求成功回调回来，才开始下一个链式的请求
 */
/* lzy注170717：
 用于管理有相互依赖的网络请求，它实际上最终可以用来管理多个拓扑排序后的网络请求。
 
 例如，我们有一个需求，需要用户在注册时，先发送注册的 Api，
 然后 : * 如果注册成功，再发送读取用户信息的 Api。并且，读取用户信息的 Api 需要使用注册成功返回的用户 id 号。
 * 如果注册失败，则不发送读取用户信息的 Api 了。
 
 以下是具体的代码示例，在示例中，我们在 sendChainRequest 方法中设置好了 Api 相互的依赖，然后。 我们就可以通过 chainRequestFinished 回调来处理所有网络请求都发送成功的逻辑了。如果有任何其中一个网络请求失败了，则会触发 chainRequestFailed 回调。
 
 - (void)sendChainRequest {
 RegisterApi *reg = [[RegisterApi alloc] initWithUsername:@"username" password:@"password"];
 YTKChainRequest *chainReq = [[YTKChainRequest alloc] init];
 
 [chainReq addRequest:reg callback:^(YTKChainRequest *chainRequest, YTKBaseRequest *baseRequest) {
 
     RegisterApi *result = (RegisterApi *)baseRequest;
     NSString *userId = [result userId];
     
     GetUserInfoApi *api = [[GetUserInfoApi alloc] initWithUserId:userId];
     
     [chainRequest addRequest:api callback:nil];
 
 }];
 
 chainReq.delegate = self;
 
 // start to send request
 [chainReq start];
 
 }
 
 - (void)chainRequestFinished:(YTKChainRequest *)chainRequest {
 // all requests are done
 }
 
 - (void)chainRequestFailed:(YTKChainRequest *)chainRequest failedBaseRequest:(YTKBaseRequest*)request {
 // some one of request is failed
 }
 */
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class YTKChainRequest;
@class YTKBaseRequest;
@protocol YTKRequestAccessory;

///  The YTKChainRequestDelegate protocol defines several optional methods you can use
///  to receive network-related messages. All the delegate methods will be called
///  on the main queue. Note the delegate methods will be called when all the requests
///  of chain request finishes.
/* lzy注170718：
 YTKChainRequestDelegate协议定义了几个可选方法，你可以使用这些可选方法来接收网络相关的消息。
 所有的delegate方法都将在主线程回调。
 注意，delegate方法只会，在链式请求中所有的请求结束之后，回调这些时间点。
 */
@protocol YTKChainRequestDelegate <NSObject>

@optional
///  Tell the delegate that the chain request has finished successfully.
///
///  @param chainRequest The corresponding chain request.
/* lzy注170718：
 通知delegate对象，链式请求已经成功完成。
 */
- (void)chainRequestFinished:(YTKChainRequest *)chainRequest;

///  Tell the delegate that the chain request has failed.
///
///  @param chainRequest The corresponding chain request.
///  @param request      First failed request that causes the whole request to fail.
/* lzy注170718：
 通知delegate对象，链式请求失败了。
 */
- (void)chainRequestFailed:(YTKChainRequest *)chainRequest failedBaseRequest:(YTKBaseRequest*)request;

@end

/* lzy注170717：
 声明链式请求的回调block
 */
typedef void (^YTKChainCallback)(YTKChainRequest *chainRequest, YTKBaseRequest *baseRequest);

// 这段注释应该是作者从batchRequest中拿来的，没怎么改。有些应该指代本类，而不是batch。
///  YTKBatchRequest can be used to chain several YTKRequest so that one will only starts after another finishes.
///  Note that when used inside YTKChainRequest, a single YTKRequest will have its own callback and delegate
///  cleared, in favor of the batch request callback.
/* lzy注170718：
 本类可用于 链接一组YTKRequest，使得其中的某个YTKRequest只会在另一个请求结束之后，才会开始。
 注意，在YTKChainRequest中使用YTKRequest时，YTKRequest有自己的callback和delegate，这些回调是用于支持本类汇总请求结果的。
 */
@interface YTKChainRequest : NSObject

///  All the requests are stored in this array.
/* lzy注170718：
 所有的请求都被有序的存储在这个数组中。
 */
- (NSArray<YTKBaseRequest *> *)requestArray;

///  The delegate object of the chain request. Default is nil.
/* lzy注170718：
  本类实例的delegate对象。默认为nil。
 */
@property (nonatomic, weak, nullable) id<YTKChainRequestDelegate> delegate;

///  This can be used to add several accossories object. Note if you use `addAccessory` to add acceesory
///  this array will be automatically created. Default is nil.
/* lzy注170718：
 可以用于添加多个『请求配件』对象。
 注意，如果你使用『addAccessory』方法添加『请求配件』，这个数组将会被自动创建。
 */
@property (nonatomic, strong, nullable) NSMutableArray<id<YTKRequestAccessory>> *requestAccessories;

///  Convenience method to add request accessory. See also `requestAccessories`.
/* lzy注170718：
 给本类请求『请求配件』的快捷方法。参看『requestAccessories』属性。
 */
- (void)addAccessory:(id<YTKRequestAccessory>)accessory;

///  Start the chain request, adding first request in the chain to request queue.
/* lzy注170718：
 通过把链式请求数组中的第一个请求添加到请求队列中，来开始这个链式请求
 */
- (void)start;

///  Stop the chain request. Remaining request in chain will be cancelled.
/* lzy注170718：
 停止这个链式请求。链式请求中剩下的请求将会被cancelled。
 */
- (void)stop;

///  Add request to request chain.
///
///  @param request  The request to be chained.
///  @param callback The finish callback
/* lzy注170718：
 给链式请求对象，添加一个带完成回调的请求。
 */
- (void)addRequest:(YTKBaseRequest *)request callback:(nullable YTKChainCallback)callback;

@end

NS_ASSUME_NONNULL_END
