// Fill out your copyright notice in the Description page of Project Settings.

#include "ThrowableItem.h"
#include "Components/StaticMeshComponent.h"
#include "Main.h"
#include "SpecialEffect.h"


// Sets default values
AThrowableItem::AThrowableItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ThrownMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThrownMesh"));
	SetRootComponent(ThrownMesh);

	StartingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartingMesh"));
	StartingMesh->SetupAttachment(GetRootComponent());

	EffectDelayTime = 5.f;

}

// Called when the game starts or when spawned
void AThrowableItem::BeginPlay()
{
	Super::BeginPlay();
	ThrownMesh->SetVisibility(false);
	StartingMesh->SetVisibility(true);
}

// Called every frame
void AThrowableItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AThrowableItem::PinPulled()
{
	StartingMesh->DestroyComponent();
	ThrownMesh->SetVisibility(true);
	GetWorldTimerManager().SetTimer(EffectBeginTimer, this, &AThrowableItem::SpawnEffect, EffectDelayTime);
}

void AThrowableItem::SpawnEffect()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ASpecialEffect* SpecialEffect = GetWorld()->SpawnActor<ASpecialEffect>(Effect, GetActorLocation(), FRotator(0.f), SpawnParams);
	if (SpecialEffect)
		SpecialEffect->SetMainReference(RefToMain);
	if (RefToMain)
		RefToMain->MakeNoiseAsNullInstigator(1.f, GetActorLocation());
	Destroy();
}

