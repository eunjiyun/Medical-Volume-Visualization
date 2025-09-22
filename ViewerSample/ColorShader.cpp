//#include "stdafx.h"
#include "ColorShader.h"


ColorShader::ColorShader()
{

}


ColorShader::ColorShader(const ColorShader& other)
{
}


ColorShader::~ColorShader()
{
}


bool ColorShader::Initialize(ID3D11Device* device, HWND hwnd)
{
	// ?類ㅼ젎 獄???? ?癒?뵠?遺? ?λ뜃由?酉鍮??덈뼄.
	return InitializeShader(device, hwnd, (WCHAR*)L"../ViewerSample/color.vs", (WCHAR*)L"../ViewerSample/color.ps");
}


void ColorShader::Shutdown()
{
	// 甕곌쑵???獄???? ?癒?뵠?遺? ?온??ㅻ쭆 揶쏆빘猿쒐몴??ル굝利??몃빍??
	ShutdownShader();
}


bool ColorShader::Render(ID3D11DeviceContext* deviceContext, int indexCount,
	XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	// ???쐭筌띻낯肉???????怨쀬뵠??筌띲끆而?癰궰??? ??쇱젟??몃빍??
	if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix))
	{
		return false;
	}

	// ??쇱젟??甕곌쑵?곭몴??怨쀬뵠?遺얠쨮 ???쐭筌띻낱釉??
	RenderShader(deviceContext, indexCount);

	return true;
}


bool ColorShader::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	ID3D10Blob* errorMessage = nullptr;

	// 甕곌쑵????癒?뵠???꾨뗀諭띄몴??뚮똾???노립??
	ID3D10Blob* vertexShaderBuffer = nullptr;
	if (FAILED(D3DCompileFromFile(vsFilename, NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage)))
	{
		// ?怨쀬뵠???뚮똾?????쎈솭????살첒筌롫뗄?놅쭪????곗뮆???몃빍??
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// ?뚮똾?????살첒揶쎛 ?袁⑤빍??겹늺 ?怨쀬뵠?????뵬??筌≪뼚??????용뮉 野껋럩???낅빍??
		else
		{
			OutputDebugStringW(vsFilename);

			//250922
			//MessageBox(hwnd, (LPCSTR)vsFilename, (LPCSTR)L"Missing Shader File", MB_OK);
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// ??? ?癒?뵠???꾨뗀諭띄몴??뚮똾???노립??
	ID3D10Blob* pixelShaderBuffer = nullptr;
	if (FAILED(D3DCompileFromFile(psFilename, NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage)))
	{
		// ?怨쀬뵠???뚮똾?????쎈솭????살첒筌롫뗄?놅쭪????곗뮆???몃빍??
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// ?뚮똾?????살첒揶쎛 ?袁⑤빍??겹늺 ?怨쀬뵠?????뵬??筌≪뼚??????용뮉 野껋럩???낅빍??
		else
		{
			//250922
			//MessageBox(hwnd, (LPCSTR)psFilename, (LPCSTR)L"Missing Shader File", MB_OK);
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// 甕곌쑵?곫에?????類ㅼ젎 ?怨쀬뵠?遺? ??밴쉐??뺣뼄.
	if (FAILED(device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader)))
	{
		return false;
	}

	// 甕곌쑵??癒?퐣 ??? ?癒?뵠?遺? ??밴쉐??몃빍??
	if (FAILED(device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader)))
	{
		return false;
	}

	// ?類ㅼ젎 ??낆젾 ??됱뵠?袁⑹뜍 ?닌듼쒙㎗?? ??쇱젟??몃빍??
	// ????쇱젟?? ModelClass?? ?怨쀬뵠?遺우벥 VertexType ?닌듼?? ??깊뒄??곷튊??몃빍??
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// ??됱뵠?袁⑹뜍???遺용꺖 ??? 揶쎛?紐꾩긿??덈뼄.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// ?類ㅼ젎 ??낆젾 ??됱뵠?袁⑹뜍??筌띾슢踰??덈뼄.
	if (FAILED(device->CreateInputLayout(polygonLayout, numElements,
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout)))
	{
		return false;
	}

	// ????곴맒 ?????? ??낅뮉 ?類ㅼ젎 ?怨쀬뵠????곗쒔?? ??? ?怨쀬뵠??甕곌쑵?곭몴???곸젫??몃빍??
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// ?類ㅼ젎 ?怨쀬뵠?遺용퓠 ??덈뮉 ??곗졊 ?怨몃땾 甕곌쑵????닌듼쒙㎗?? ?臾믨쉐??몃빍??
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// ?怨몃땾 甕곌쑵??????怨? 筌띾슢諭?????????쇰퓠???類ㅼ젎 ?怨쀬뵠???怨몃땾 甕곌쑵????臾롫젏??????뉗쓺 ??몃빍??
	if (FAILED(device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer)))
	{
		return false;
	}

	return true;
}


void ColorShader::ShutdownShader()
{
	// ??곗졊 ?怨몃땾 甕곌쑵?곭몴???곸젫??몃빍??
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// ??됱뵠?袁⑹뜍????곸젫??몃빍??
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// ??? ?癒?뵠?遺? ??곸젫??몃빍??
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// 甕곌쑵????癒?뵠?遺? ??곸젫??몃빍??
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}
}


void ColorShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	// ?癒?쑎 筌롫뗄?놅쭪????곗뮆?곤㎕?뚮퓠 ??뽯뻻??몃빍??
	OutputDebugStringA(reinterpret_cast<const char*>(errorMessage->GetBufferPointer()));

	// ?癒?쑎 筌롫뗄苑?쭪???獄쏆꼹???몃빍??
	errorMessage->Release();
	errorMessage = 0;

	//250922
	// ?뚮똾????癒?쑎揶쎛 ??됱벉????밸씜 筌롫뗄苑?쭪?嚥????젻餓λ씭???
	//MessageBox(hwnd, (LPCSTR)L"Error compiling shader.", (LPCSTR)shaderFilename, MB_OK);
	MessageBox(hwnd, L"Error compiling shader.", shaderFilename, MB_OK);
}


bool ColorShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	// ??곗졊??transpose??뤿연 ?怨쀬뵠?遺용퓠???????????뉗쓺 ??몃빍??
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// ?怨몃땾 甕곌쑵?????곸뒠????????덈즲嚥??醫됲닋??덈뼄.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	// ?怨몃땾 甕곌쑵????怨쀬뵠?怨쀫퓠 ????????怨? 揶쎛?紐꾩긿??덈뼄.
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	// ?怨몃땾 甕곌쑵?????곗졊??癰귣벊沅??몃빍??
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// ?怨몃땾 甕곌쑵????醫됲닊????곕빍??
	deviceContext->Unmap(m_matrixBuffer, 0);

	// ?類ㅼ젎 ?怨쀬뵠?遺용퓠??뽰벥 ?怨몃땾 甕곌쑵????袁⑺뒄????쇱젟??몃빍??
	unsigned bufferNumber = 0;

	// 筌띾뜆?筌띾맩?앮에??類ㅼ젎 ?怨쀬뵠?遺우벥 ?怨몃땾 甕곌쑵?곭몴?獄쏅뗀??揶쏅??앮에?獄쏅떽???덈뼄.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	return true;
}


void ColorShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// ?類ㅼ젎 ??낆젾 ??됱뵠?袁⑹뜍????쇱젟??몃빍??
	deviceContext->IASetInputLayout(m_layout);

	// ??⑥퍟?類ㅼ뱽 域밸챶???類ㅼ젎 ?怨쀬뵠?遺? ??? ?怨쀬뵠?遺? ??쇱젟??몃빍??
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// ??⑥퍟?類ㅼ뱽 域밸챶???덈뼄.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}