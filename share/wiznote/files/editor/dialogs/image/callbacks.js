/**
 * Created by JetBrains WebStorm.
 * User: taoqili
 * Date: 11-9-29
 * Time: 下午3:50
 * To change this template use File | Settings | File Templates.
 */
/**
 * 选择文件后的回调函数
 * @param	Array
 */
function selectFileCallback(selectFiles){
    // 数组里单个元素为Object，{index:在多图上传的索引号, name:文件名, size:文件大小}
    // 其中size单位为Byte
    console.log("选择了如下文件：");
    var obj;
    for(var i = 0, iLen = selectFiles.length; i < iLen; i++){
        obj = selectFiles[i];
        console.log(obj.index, obj.name, obj.size);
    }
    console.log("===================================");
}
/**
 * 文件大小超出时的回调函数
 * @param	Object
 */
function exceedFileCallback(file){
    // 参数为Object，{index:在多图上传的索引号, name:文件名, size:文件大小}
    // 其中size单位为Byte
    console.log("文件超出大小限制：");
    console.log(file.index, file.name, file.size);
    console.log("===================================");
}
/**
 * 删除文件后的回调函数
 * @param	Array
 */
function deleteFileCallback(delFiles){
    // 数组里单个元素为Object，{index:在多图上传的索引号, name:文件名, size:文件大小}
    // 其中size单位为Byte
    console.log("删除了如下文件：");
    var obj;
    for(var i = 0, iLen = delFiles.length; i < iLen; i++){
        obj = delFiles[i];
        console.log(obj.index, obj.name, obj.size);
    }
    console.log("===================================");
}
/**
 * 开始上传单个文件的回调函数
 * @param	Object
 */
function startUploadCallback(file){
    console.log("开始上传如下文件：");
    console.log(file.name, file.size);
    console.log("===================================");
}
/**
 * 单个文件上传完成的回调函数
 * @param	Object/String	服务端返回啥，参数就是啥
 */
	function uploadCompleteCallback(data){
		console.log("上传成功", data);
        console.log("===================================");
	}
 /**
  * 单个文件上传失败的回调函数
  * @param	Object/String	服务端返回啥，参数就是啥
  */
	function uploadErrorCallback(data){
		console.log("上传失败", data);
        console.log("===================================");
	}
 /**
  * 全部上传完成的回调函数
  */
	function allCompleteCallback(){
		console.log("全部上传成功");
        console.log("===================================");
	}
