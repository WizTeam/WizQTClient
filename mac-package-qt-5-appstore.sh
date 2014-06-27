# 挂载的volumn的名称
volumn_name='wiznote-disk'
# 挂载点，一般不用修改，只配置volumn_name即可
volumn_path="/Volumes/$volumn_name"

# 以下参数只是用于自定义脚本的行为
package_home="./macos-package"
package_output_path="$HOME"



# compile
mkdir ../WizQTClient-Release-QT5
rm -rf ../WizQTClient-Release-QT5/* && \
cd ../WizQTClient-Release-QT5 && \
cmake -DWIZNOTE_USE_QT5=YES -DCMAKE_BUILD_TYPE=Release -UPDATE_TRANSLATIONS=YES -DAPPSTORE_BUILD=YES -DCMAKE_PREFIX_PATH=~/usr/local/Qt-5.3.0/lib/cmake ../WizQTClient && \
make -j5 && \
productbuild --component WizNote.app /Applications --sign "3rd Party Mac Developer Installer: Wei Shijun" wiznote.pkg


